#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <signal.h>
#include <stdint.h>
#include "iic_lib.h"

#define MAP_SIZE 4096

#define TEMP_UIO_PATH "/dev/uio0"

static int read_temps(char *uiod_iic)
{
    void *ip_ptr;
    int fd;
    int read_oke = 0;

    unsigned int rd_val;

    int m_board_temp = -1;
    int m_chip_temp = -1;

    u8 Buffer[2];
    u8 slaveAddr;
    // open uio
    fd = open(uiod_iic, O_RDWR);

    if (fd < 1)
    {
        perror("Cannot open IIC \n");
        return 0;
    }

    // mmap
    ip_ptr = mmap(NULL, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    XIic_DynInit(ip_ptr);

    if (ip_ptr == MAP_FAILED)
    {

        perror("mmap IIC failed \n");
        return 0;
    }

    u8 sendBuf[3];

    slaveAddr = 0x4E;
    // read board temp

    sendBuf[0] = 0x00;
    if (XIic_Send(ip_ptr, slaveAddr, sendBuf, 1, XIIC_STOP, 1000) > 0)
    {
        // sleep(5);
        // printf("STATUS = 0x%x\n", (u8) XIic_ReadReg(ip_ptr, 0x104));

        for (int i = 0; i < 1; ++i)
        {
            memset(Buffer, 0, 2);
            if (XIic_Recv(ip_ptr, slaveAddr, Buffer, 1, XIIC_STOP, 1000) > 0)
            {
                usleep(1000);
                if (Buffer[0] > -20 && Buffer[0] < 150)
                {
                    int m_board_temp = Buffer[0];
                    printf("BOARD TEMP = %d\n", m_board_temp);
                    break;
                }
            }
        }

        // change to read chip temp
        sendBuf[0] = 0x01;
        XIic_Send(ip_ptr, slaveAddr, sendBuf, 1, XIIC_STOP, 1000);
        usleep(1000);
        for (int i = 0; i < 1; ++i)
        {
            memset(Buffer, 0, 2);
            if (XIic_Recv(ip_ptr, slaveAddr, Buffer, 1, XIIC_STOP, 1000) > 0)
            {
                usleep(1000);
                if (Buffer[0] > 0 && Buffer[0] < 150)
                {
                    int m_chip_temp = Buffer[0];
                    printf("BOARD TEMP = %d\n", m_chip_temp);
                    break;
                }
            }
        }
    }

    munmap(ip_ptr, MAP_SIZE);
    close(fd);

    return 1;
}

int main(int argc, char const *argv[])
{
    read_temps(TEMP_UIO_PATH);
    return 0;
}