/***************************** Include Files *******************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <signal.h>
#include <stdint.h>
#include "iic_lib.h"

/************************** Constant Definitions ***************************/

/**************************** Type Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *******************/

/************************** Function Prototypes ****************************/

static unsigned RecvData(void *ip_ptr, u8 *BufferPtr,
                         unsigned ByteCount, u8 Option, u32 timeout);
static unsigned SendData(void *ip_ptr, u8 *BufferPtr,
                         unsigned ByteCount, u8 Option, u32 timeout);

static unsigned DynRecvData(void *ip_ptr, u8 *BufferPtr, u8 ByteCount, u32 timeout);
static unsigned DynSendData(void *ip_ptr, u8 *BufferPtr,
                            u8 ByteCount, u8 Option, u32 timeout);

/************************** Variable Definitions **************************/

/****************************************************************************/
/**
 * Receive data as a master on the IIC bus.  This function receives the data
 * using polled I/O and blocks until the data has been received. It only
 * supports 7 bit addressing mode of operation. This function returns zero
 * if bus is busy.
 *
 * @param        ip_ptr contains the base address of the IIC device.
 * @param        Address contains the 7 bit IIC address of the device to send the
 *               specified data to.
 * @param        BufferPtr points to the data to be sent.
 * @param        ByteCount is the number of bytes to be sent.
 * @param        Option indicates whether to hold or free the bus after reception
 *               of data, XIIC_STOP = end with STOP condition,
 *               XIIC_REPEATED_START = don't end with STOP condition.
 *
 * @return       The number of bytes received.
 *
 * @note         None.
 *
 ******************************************************************************/
unsigned XIic_Recv(void *ip_ptr, u8 Address,
                   u8 *BufferPtr, unsigned ByteCount, u8 Option, u32 timeout)
{
    u32 CntlReg;
    unsigned RemainingByteCount;
    volatile u32 StatusReg;

    /* Tx error is enabled in case the address (7 or 10) has no device to
     * answer with Ack. When only one byte of data, must set NO ACK before
     * address goes out therefore Tx error must not be enabled as it will go
     * off immediately and the Rx full interrupt will be checked.  If full,
     * then the one byte was received and the Tx error will be disabled
     * without sending an error callback msg
     */
    XIic_ClearIisr(ip_ptr,
                   XIIC_INTR_RX_FULL_MASK | XIIC_INTR_TX_ERROR_MASK |
                       XIIC_INTR_ARB_LOST_MASK);

    /* Set receive FIFO occupancy depth for 1 byte (zero based) */
    XIic_WriteReg(ip_ptr, XIIC_RFD_REG_OFFSET, 0);

    /* Check to see if already Master on the Bus.
     * If Repeated Start bit is not set send Start bit by setting MSMS bit
     * else Send the address
     */
    CntlReg = XIic_ReadReg(ip_ptr, XIIC_CR_REG_OFFSET);
    if ((CntlReg & XIIC_CR_REPEATED_START_MASK) == 0)
    {
        /* 7 bit slave address, send the address for a read operation
         * and set the state to indicate the address has been sent
         */
        XIic_Send7BitAddress(ip_ptr, Address,
                             XIIC_READ_OPERATION);

        /* MSMS gets set after putting data in FIFO. Start the master
         * receive operation by setting CR Bits MSMS to Master, if the
         * buffer is only one byte, then it should not be acknowledged
         * to indicate the end of data
         */
        CntlReg = XIIC_CR_MSMS_MASK | XIIC_CR_ENABLE_DEVICE_MASK;
        if (ByteCount == 1)
        {
            CntlReg |= XIIC_CR_NO_ACK_MASK;
        }

        /* Write out the control register to start receiving data and
         * call the function to receive each byte into the buffer
         */
        XIic_WriteReg(ip_ptr, XIIC_CR_REG_OFFSET, CntlReg);

        /* Clear the latched interrupt status for the bus not busy bit
         * which must be done while the bus is busy
         */
        StatusReg = XIic_ReadReg(ip_ptr, XIIC_SR_REG_OFFSET);
        u32 timeout_cnt = 0;
        while ((StatusReg & XIIC_SR_BUS_BUSY_MASK) == 0)
        {
            StatusReg = XIic_ReadReg(ip_ptr,
                                     XIIC_SR_REG_OFFSET);
            timeout_cnt++;
            if (timeout_cnt > timeout)
            {
                return 0;
            }
        }

        XIic_ClearIisr(ip_ptr, XIIC_INTR_BNB_MASK);
    }
    else
    {
        /* Before writing 7bit slave address the Direction of Tx bit
         * must be disabled
         */
        CntlReg &= ~XIIC_CR_DIR_IS_TX_MASK;
        if (ByteCount == 1)
        {
            CntlReg |= XIIC_CR_NO_ACK_MASK;
        }
        XIic_WriteReg(ip_ptr, XIIC_CR_REG_OFFSET, CntlReg);
        /* Already owns the Bus indicating that its a Repeated Start
         * call. 7 bit slave address, send the address for a read
         * operation and set the state to indicate the address has been
         * sent
         */
        XIic_Send7BitAddress(ip_ptr, Address,
                             XIIC_READ_OPERATION);
    }
    /* Try to receive the data from the IIC bus */

    RemainingByteCount = RecvData(ip_ptr, BufferPtr,
                                  ByteCount, Option, 10000);

    CntlReg = XIic_ReadReg(ip_ptr, XIIC_CR_REG_OFFSET);
    if ((CntlReg & XIIC_CR_REPEATED_START_MASK) == 0)
    {
        /* The receive is complete, disable the IIC device if the Option
         * is to release the Bus after Reception of data and return the
         * number of bytes that was received
         */
        XIic_WriteReg(ip_ptr, XIIC_CR_REG_OFFSET, 0);
    }

    /* Wait until I2C bus is freed, exit if timed out. */
    if (XIic_WaitBusFree(ip_ptr) != XST_SUCCESS)
    {
        return 0;
    }

    /* Return the number of bytes that was received */
    return ByteCount - RemainingByteCount;
}

/******************************************************************************
 *
 * Receive the specified data from the device that has been previously addressed
 * on the IIC bus.  This function assumes that the 7 bit address has been sent
 * and it should wait for the transmit of the address to complete.
 *
 * @param        ip_ptr contains the base address of the IIC device.
 * @param        BufferPtr points to the buffer to hold the data that is
 *               received.
 * @param        ByteCount is the number of bytes to be received.
 * @param        Option indicates whether to hold or free the bus after reception
 *               of data, XIIC_STOP = end with STOP condition,
 *               XIIC_REPEATED_START = don't end with STOP condition.
 *
 * @return       The number of bytes remaining to be received.
 *
 * @note
 *
 * This function does not take advantage of the receive FIFO because it is
 * designed for minimal code space and complexity.  It contains loops that
 * that could cause the function not to return if the hardware is not working.
 *
 * This function assumes that the calling function will disable the IIC device
 * after this function returns.
 *
 ******************************************************************************/
static unsigned RecvData(void *ip_ptr, u8 *BufferPtr,
                         unsigned ByteCount, u8 Option, u32 timeout)
{
    u32 CntlReg;
    u32 IntrStatusMask;
    u32 IntrStatus;

    u32 timeout_cnt = 0;

    /* Attempt to receive the specified number of bytes on the IIC bus */

    while (ByteCount > 0)
    {
        /* Setup the mask to use for checking errors because when
         * receiving one byte OR the last byte of a multibyte message an
         * error naturally occurs when the no ack is done to tell the
         * slave the last byte
         */
        if (ByteCount == 1)
        {
            IntrStatusMask =
                XIIC_INTR_ARB_LOST_MASK | XIIC_INTR_BNB_MASK;
        }
        else
        {
            IntrStatusMask =
                XIIC_INTR_ARB_LOST_MASK |
                XIIC_INTR_TX_ERROR_MASK | XIIC_INTR_BNB_MASK;
        }

        /* Wait for the previous transmit and the 1st receive to
         * complete by checking the interrupt status register of the
         * IPIF
         */
        while (1)
        {
            IntrStatus = XIic_ReadIisr(ip_ptr);
            if (IntrStatus & XIIC_INTR_RX_FULL_MASK)
            {
                break;
            }
            /* Check the transmit error after the receive full
             * because when sending only one byte transmit error
             * will occur because of the no ack to indicate the end
             * of the data
             */
            if (IntrStatus & IntrStatusMask)
            {
                return ByteCount;
            }
            timeout_cnt++;
            if(timeout_cnt > timeout){
                return ByteCount;
            }
        }

        CntlReg = XIic_ReadReg(ip_ptr, XIIC_CR_REG_OFFSET);

        /* Special conditions exist for the last two bytes so check for
         * them. Note that the control register must be setup for these
         * conditions before the data byte which was already received is
         * read from the receive FIFO (while the bus is throttled
         */
        if (ByteCount == 1)
        {
            if (Option == XIIC_STOP)
            {

                /* If the Option is to release the bus after the
                 * last data byte, it has already been read and
                 * no ack has been done, so clear MSMS while
                 * leaving the device enabled so it can get off
                 * the IIC bus appropriately with a stop
                 */
                XIic_WriteReg(ip_ptr, XIIC_CR_REG_OFFSET,
                              XIIC_CR_ENABLE_DEVICE_MASK);
            }
        }

        /* Before the last byte is received, set NOACK to tell the slave
         * IIC device that it is the end, this must be done before
         * reading the byte from the FIFO
         */
        if (ByteCount == 2)
        {
            /* Write control reg with NO ACK allowing last byte to
             * have the No ack set to indicate to slave last byte
             * read
             */
            XIic_WriteReg(ip_ptr, XIIC_CR_REG_OFFSET,
                          CntlReg | XIIC_CR_NO_ACK_MASK);
        }

        /* Read in data from the FIFO and unthrottle the bus such that
         * the next byte is read from the IIC bus
         */

/*        if (ByteCount == 2)
        {
            //printf("Low byte = 0x%x\n", (u8)XIic_ReadReg(ip_ptr, XIIC_DRR_REG_OFFSET));
        }

        if (ByteCount == 1)
        {
            //printf("High byte = 0x%x\n", (u8)XIic_ReadReg(ip_ptr, XIIC_DRR_REG_OFFSET));
        }*/
        // printf("rd val = 0x%x\n", (u8) XIic_ReadReg(ip_ptr, XIIC_DRR_REG_OFFSET));

        *BufferPtr++ = (u8)XIic_ReadReg(ip_ptr,
                                        XIIC_DRR_REG_OFFSET);

        if ((ByteCount == 1) && (Option == XIIC_REPEATED_START))
        {

            /* RSTA bit should be set only when the FIFO is
             * completely Empty.
             */
            XIic_WriteReg(ip_ptr, XIIC_CR_REG_OFFSET,
                          XIIC_CR_ENABLE_DEVICE_MASK | XIIC_CR_MSMS_MASK | XIIC_CR_REPEATED_START_MASK);
        }

        /* Clear the latched interrupt status so that it will be updated
         * with the new state when it changes, this must be done after
         * the receive register is read
         */
        XIic_ClearIisr(ip_ptr, XIIC_INTR_RX_FULL_MASK |
                                   XIIC_INTR_TX_ERROR_MASK |
                                   XIIC_INTR_ARB_LOST_MASK);
        ByteCount--;
    }

    if (Option == XIIC_STOP)
    {

        /* If the Option is to release the bus after Reception of data,
         * wait for the bus to transition to not busy before returning,
         * the IIC device cannot be disabled until this occurs. It
         * should transition as the MSMS bit of the control register was
         * cleared before the last byte was read from the FIFO
         */
        timeout_cnt = 0;
        while (1)
        {
            if (XIic_ReadIisr(ip_ptr) & XIIC_INTR_BNB_MASK)
            {
                break;
            }
            timeout_cnt++;
            if (timeout_cnt > timeout)
            {
                break;
            }
        }
    }

    return ByteCount;
}

/****************************************************************************/
/**
 * Send data as a master on the IIC bus.  This function sends the data
 * using polled I/O and blocks until the data has been sent. It only supports
 * 7 bit addressing mode of operation.  This function returns zero
 * if bus is busy.
 *
 * @param        ip_ptr contains the base address of the IIC device.
 * @param        Address contains the 7 bit IIC address of the device to send the
 *               specified data to.
 * @param        BufferPtr points to the data to be sent.
 * @param        ByteCount is the number of bytes to be sent.
 * @param        Option indicates whether to hold or free the bus after
 *               transmitting the data.
 *
 * @return       The number of bytes sent.
 *
 * @note         None.
 *
 ******************************************************************************/
unsigned XIic_Send(void *ip_ptr, u8 Address,
                   u8 *BufferPtr, unsigned ByteCount, u8 Option, u32 timeout)
{
    unsigned RemainingByteCount;
    u32 ControlReg;
    volatile u32 StatusReg;

    /* Wait until I2C bus is freed, exit if timed out. */
    if (XIic_WaitBusFree(ip_ptr) != XST_SUCCESS)
    {
        return 0;
    }

    /* Check to see if already Master on the Bus.
     * If Repeated Start bit is not set send Start bit by setting
     * MSMS bit else Send the address.
     */
    ControlReg = XIic_ReadReg(ip_ptr, XIIC_CR_REG_OFFSET);
    if ((ControlReg & XIIC_CR_REPEATED_START_MASK) == 0)
    {
        /*
         * Put the address into the FIFO to be sent and indicate
         * that the operation to be performed on the bus is a
         * write operation
         */
        XIic_Send7BitAddress(ip_ptr, Address,
                             XIIC_WRITE_OPERATION);
        /* Clear the latched interrupt status so that it will
         * be updated with the new state when it changes, this
         * must be done after the address is put in the FIFO
         */
        XIic_ClearIisr(ip_ptr, XIIC_INTR_TX_EMPTY_MASK |
                                   XIIC_INTR_TX_ERROR_MASK |
                                   XIIC_INTR_ARB_LOST_MASK);

        /*
         * MSMS must be set after putting data into transmit FIFO,
         * indicate the direction is transmit, this device is master
         * and enable the IIC device
         */
        XIic_WriteReg(ip_ptr, XIIC_CR_REG_OFFSET,
                      XIIC_CR_MSMS_MASK | XIIC_CR_DIR_IS_TX_MASK |
                          XIIC_CR_ENABLE_DEVICE_MASK);

        /*
         * Clear the latched interrupt
         * status for the bus not busy bit which must be done while
         * the bus is busy
         */
        StatusReg = XIic_ReadReg(ip_ptr, XIIC_SR_REG_OFFSET);
        u32 timeout_cnt = 0;
        while ((StatusReg & XIIC_SR_BUS_BUSY_MASK) == 0)
        {
            StatusReg = XIic_ReadReg(ip_ptr,
                                     XIIC_SR_REG_OFFSET);
            timeout_cnt++;
            if(timeout_cnt > timeout){
               // printf("debug 2\n");
                return 0;
            }
        }

        XIic_ClearIisr(ip_ptr, XIIC_INTR_BNB_MASK);
    }
    else
    {
        /*
         * Already owns the Bus indicating that its a Repeated Start
         * call. 7 bit slave address, send the address for a write
         * operation and set the state to indicate the address has
         * been sent.
         */
        XIic_Send7BitAddress(ip_ptr, Address,
                             XIIC_WRITE_OPERATION);
    }

    /* Send the specified data to the device on the IIC bus specified by the
     * the address
     */
    RemainingByteCount = SendData(ip_ptr, BufferPtr,
                                  ByteCount, Option, 10000);

    ControlReg = XIic_ReadReg(ip_ptr, XIIC_CR_REG_OFFSET);
    if ((ControlReg & XIIC_CR_REPEATED_START_MASK) == 0)
    {
        /*
         * The Transmission is completed, disable the IIC device if
         * the Option is to release the Bus after transmission of data
         * and return the number of bytes that was received. Only wait
         * if master, if addressed as slave just reset to release
         * the bus.
         */
        if ((ControlReg & XIIC_CR_MSMS_MASK) != 0)
        {
            XIic_WriteReg(ip_ptr, XIIC_CR_REG_OFFSET,
                          (ControlReg & ~XIIC_CR_MSMS_MASK));
        }

        if ((XIic_ReadReg(ip_ptr, XIIC_SR_REG_OFFSET) &
             XIIC_SR_ADDR_AS_SLAVE_MASK) != 0)
        {
            XIic_WriteReg(ip_ptr, XIIC_CR_REG_OFFSET, 0);
        }
        else
        {
            u32 timeout_cnt = 0;
            StatusReg = XIic_ReadReg(ip_ptr,
                                     XIIC_SR_REG_OFFSET);
            while ((StatusReg & XIIC_SR_BUS_BUSY_MASK) != 0)
            {
                StatusReg = XIic_ReadReg(ip_ptr,
                                         XIIC_SR_REG_OFFSET);
                timeout_cnt++;
                if (timeout_cnt > timeout)
                {
                  //  printf("debug 2\n");
                    return 0;
                }
            }
        }
    }

    return ByteCount - RemainingByteCount;
}

/******************************************************************************
 *
 * Send the specified buffer to the device that has been previously addressed
 * on the IIC bus.  This function assumes that the 7 bit address has been sent
 * and it should wait for the transmit of the address to complete.
 *
 * @param        ip_ptr contains the base address of the IIC device.
 * @param        BufferPtr points to the data to be sent.
 * @param        ByteCount is the number of bytes to be sent.
 * @param        Option indicates whether to hold or free the bus after
 *               transmitting the data.
 *
 * @return       The number of bytes remaining to be sent.
 *
 * @note
 *
 * This function does not take advantage of the transmit FIFO because it is
 * designed for minimal code space and complexity.  It contains loops that
 * that could cause the function not to return if the hardware is not working.
 *
 ******************************************************************************/
static unsigned SendData(void *ip_ptr, u8 *BufferPtr,
                         unsigned ByteCount, u8 Option, u32 timeout)
{
    u32 IntrStatus;
    u32 timeout_cnt = 0;

    /*
     * Send the specified number of bytes in the specified buffer by polling
     * the device registers and blocking until complete
     */
    while (ByteCount > 0)
    {
        /*
         * Wait for the transmit to be empty before sending any more
         * data by polling the interrupt status register
         */
        while (1)
        {
            IntrStatus = XIic_ReadIisr(ip_ptr);

            if (IntrStatus & (XIIC_INTR_TX_ERROR_MASK |
                              XIIC_INTR_ARB_LOST_MASK |
                              XIIC_INTR_BNB_MASK))
            {
                return ByteCount;
            }

            if (IntrStatus & XIIC_INTR_TX_EMPTY_MASK)
            {
                break;
            }

            timeout_cnt++;
            if(timeout_cnt > timeout){
             //   printf("debug 4\n");
                return ByteCount;
            }
        }
        /* If there is more than one byte to send then put the
         * next byte to send into the transmit FIFO
         */
        if (ByteCount > 1)
        {
            XIic_WriteReg(ip_ptr, XIIC_DTR_REG_OFFSET,
                          *BufferPtr++);
        }
        else
        {
            if (Option == XIIC_STOP)
            {
                /*
                 * If the Option is to release the bus after
                 * the last data byte, Set the stop Option
                 * before sending the last byte of data so
                 * that the stop Option will be generated
                 * immediately following the data. This is
                 * done by clearing the MSMS bit in the
                 * control register.
                 */
                XIic_WriteReg(ip_ptr, XIIC_CR_REG_OFFSET,
                              XIIC_CR_ENABLE_DEVICE_MASK |
                                  XIIC_CR_DIR_IS_TX_MASK);
            }

            /*
             * Put the last byte to send in the transmit FIFO
             */
            XIic_WriteReg(ip_ptr, XIIC_DTR_REG_OFFSET,
                          *BufferPtr++);

            if (Option == XIIC_REPEATED_START)
            {
                XIic_ClearIisr(ip_ptr,
                               XIIC_INTR_TX_EMPTY_MASK);
                /*
                 * Wait for the transmit to be empty before
                 * setting RSTA bit.
                 */
                while (1)
                {
                    IntrStatus =
                        XIic_ReadIisr(ip_ptr);
                    if (IntrStatus &
                        XIIC_INTR_TX_EMPTY_MASK)
                    {
                        /*
                         * RSTA bit should be set only
                         * when the FIFO is completely
                         * Empty.
                         */
                        XIic_WriteReg(ip_ptr,
                                      XIIC_CR_REG_OFFSET,
                                      XIIC_CR_REPEATED_START_MASK |
                                          XIIC_CR_ENABLE_DEVICE_MASK |
                                          XIIC_CR_DIR_IS_TX_MASK |
                                          XIIC_CR_MSMS_MASK);
                        break;
                    }
                    timeout_cnt++;
                    if (timeout_cnt > timeout)
                    {
                       // printf("debug 5\n");
                        return ByteCount;
                    }
                }
            }
        }

        /*
         * Clear the latched interrupt status register and this must be
         * done after the transmit FIFO has been written to or it won't
         * clear
         */
        XIic_ClearIisr(ip_ptr, XIIC_INTR_TX_EMPTY_MASK);

        /*
         * Update the byte count to reflect the byte sent and clear
         * the latched interrupt status so it will be updated for the
         * new state
         */
        ByteCount--;
    }

    if (Option == XIIC_STOP)
    {
        /*
         * If the Option is to release the bus after transmission of
         * data, Wait for the bus to transition to not busy before
         * returning, the IIC device cannot be disabled until this
         * occurs. Note that this is different from a receive operation
         * because the stop Option causes the bus to go not busy.
         */
        timeout_cnt = 0;
        while (1)
        {
            if (XIic_ReadIisr(ip_ptr) &
                XIIC_INTR_BNB_MASK)
            {
                break;
            }
            timeout_cnt++;
            if (timeout_cnt > timeout)
            {
                break;
            }
        }
    }

    return ByteCount;
}

/*****************************************************************************/
/**
 * Receive data as a master on the IIC bus. This function receives the data
 * using polled I/O and blocks until the data has been received. It only
 * supports 7 bit addressing. This function returns zero if bus is busy.
 *
 * @param        ip_ptr contains the base address of the IIC Device.
 * @param        Address contains the 7 bit IIC Device address of the device to
 *               send the specified data to.
 * @param        BufferPtr points to the data to be sent.
 * @param        ByteCount is the number of bytes to be sent. This value can't be
 *               greater than 255 and needs to be greater than 0.
 *
 * @return       The number of bytes received.
 *
 * @note         Upon entry to this function, the IIC interface needs to be
 *               already enabled in the CR register.
 *
 ******************************************************************************/
unsigned XIic_DynRecv(void *ip_ptr, u8 Address, u8 *BufferPtr, u8 ByteCount, u32 timeout)
{
    unsigned RemainingByteCount;
    u32 StatusRegister;
    u32 timeout_cnt = 0;

    /*
     * Clear the latched interrupt status so that it will be updated with
     * the new state when it changes.
     */
    XIic_ClearIisr(ip_ptr, XIIC_INTR_TX_EMPTY_MASK |
                               XIIC_INTR_TX_ERROR_MASK | XIIC_INTR_ARB_LOST_MASK);

    /*
     * Send the 7 bit slave address for a read operation and set the state
     * to indicate the address has been sent. Upon writing the address, a
     * start condition is initiated. MSMS is automatically set to master
     * when the address is written to the Fifo. If MSMS was already set,
     * then a re-start is sent prior to the address.
     */
    XIic_DynSend7BitAddress(ip_ptr, Address, XIIC_READ_OPERATION);

    /*
     * Wait for the bus to go busy.
     */
    StatusRegister = XIic_ReadReg(ip_ptr, XIIC_SR_REG_OFFSET);

    while ((StatusRegister & XIIC_SR_BUS_BUSY_MASK) != XIIC_SR_BUS_BUSY_MASK)
    {
        StatusRegister = XIic_ReadReg(ip_ptr,
                                      XIIC_SR_REG_OFFSET);
        timeout_cnt++;
        if(timeout_cnt > timeout){
            return 0;
        }
    }

    /*
     * Clear the latched interrupt status for the bus not busy bit which
     * must be done while the bus is busy.
     */
    XIic_ClearIisr(ip_ptr, XIIC_INTR_BNB_MASK);

    /*
     * Write to the Tx Fifo the dynamic stop control bit with the number of
     * bytes that are to be read over the IIC interface from the presently
     * addressed device.
     */
    XIic_DynSendStop(ip_ptr, ByteCount);

    /*
     * Receive the data from the IIC bus.
     */
    RemainingByteCount = DynRecvData(ip_ptr, BufferPtr, ByteCount, 10000);

    /* Wait until I2C bus is freed, exit if timed out. */
    if (XIic_WaitBusFree(ip_ptr) != XST_SUCCESS)
    {
        return 0;
    }

    /*
     * The receive is complete. Return the number of bytes that were
     * received.
     */
    return ByteCount - RemainingByteCount;
}

/*****************************************************************************/
/**
 * Receive the specified data from the device that has been previously addressed
 * on the IIC bus. This function assumes the following:
 * - The Rx Fifo occupancy depth has been set to its max.
 * - Upon entry, the Rx Fifo is empty.
 * - The 7 bit address has been sent.
 * - The dynamic stop and number of bytes to receive has been written to Tx
 *   Fifo.
 *
 * @param        ip_ptr contains the base address of the IIC Device.
 * @param        BufferPtr points to the buffer to hold the data that is
 *               received.
 * @param        ByteCount is the number of bytes to be received. The range of
 *               this value is greater than 0 and not higher than 255.
 *
 * @return       The number of bytes remaining to be received.
 *
 * @note         This function contains loops that could cause the function not
 *               to return if the hardware is not working.
 *
 ******************************************************************************/
static unsigned DynRecvData(void *ip_ptr, u8 *BufferPtr, u8 ByteCount, u32 timeout)
{
    u32 StatusReg;
    u32 IntrStatus;
    u32 IntrStatusMask;
    u32 timeout_cnt = 0;

    while (ByteCount > 0)
    {

        /*
         * Setup the mask to use for checking errors because when
         * receiving one byte OR the last byte of a multibyte message
         * an error naturally occurs when the no ack is done to tell
         * the slave the last byte.
         */
        if (ByteCount == 1)
        {
            IntrStatusMask =
                XIIC_INTR_ARB_LOST_MASK | XIIC_INTR_BNB_MASK;
        }
        else
        {
            IntrStatusMask =
                XIIC_INTR_ARB_LOST_MASK |
                XIIC_INTR_TX_ERROR_MASK | XIIC_INTR_BNB_MASK;
        }

        /*
         * Wait for a byte to show up in the Rx Fifo.
         */
        while (1)
        {
            IntrStatus = XIic_ReadIisr(ip_ptr);
            StatusReg = XIic_ReadReg(ip_ptr,
                                     XIIC_SR_REG_OFFSET);

            if ((StatusReg & XIIC_SR_RX_FIFO_EMPTY_MASK) !=
                XIIC_SR_RX_FIFO_EMPTY_MASK)
            {
                break;
            }
            /*
             * Check the transmit error after the receive full
             * because when sending only one byte transmit error
             * will occur because of the no ack to indicate the end
             * of the data.
             */
            if (IntrStatus & IntrStatusMask)
            {
                return ByteCount;
            }

            timeout_cnt++;
            if(timeout_cnt > timeout){
                return ByteCount;
            }

        }

        /*
         * Read in byte from the Rx Fifo. If the Fifo reached the
         * programmed occupancy depth as programmed in the Rx occupancy
         * reg, this read access will un throttle the bus such that
         * the next byte is read from the IIC bus.
         */
        *BufferPtr++ = XIic_ReadReg(ip_ptr, XIIC_DRR_REG_OFFSET);
        ByteCount--;
    }

    return ByteCount;
}

/*****************************************************************************/
/**
 * Send data as a master on the IIC bus. This function sends the data using
 * polled I/O and blocks until the data has been sent. It only supports 7 bit
 * addressing. This function returns zero if bus is busy.
 *
 * @param        ip_ptr contains the base address of the IIC Device.
 * @param        Address contains the 7 bit IIC address of the device to send the
 *               specified data to.
 * @param        BufferPtr points to the data to be sent.
 * @param        ByteCount is the number of bytes to be sent.
 * @param        Option: XIIC_STOP = end with STOP condition,
 *               XIIC_REPEATED_START = don't end with STOP condition.
 *
 * @return       The number of bytes sent.
 *
 * @note         None.
 *
 ******************************************************************************/
unsigned XIic_DynSend(void *ip_ptr, u16 Address, u8 *BufferPtr,
                      u8 ByteCount, u8 Option, u32 timeout)
{
    unsigned RemainingByteCount;
    u32 StatusRegister;
    u32 timeout_cnt = 0;

    /* Wait until I2C bus is freed, exit if timed out. */
    if (XIic_WaitBusFree(ip_ptr) != XST_SUCCESS)
    {
        return 0;
    }

    /*
     * Clear the latched interrupt status so that it will be updated with
     * the new state when it changes, this must be done after the address
     * is put in the FIFO
     */
    XIic_ClearIisr(ip_ptr, XIIC_INTR_TX_EMPTY_MASK |
                               XIIC_INTR_TX_ERROR_MASK | XIIC_INTR_ARB_LOST_MASK);

    /*
     * Put the address into the Fifo to be sent and indicate that the
     * operation to be performed on the bus is a write operation. Upon
     * writing the address, a start condition is initiated. MSMS is
     * automatically set to master when the address is written to the Fifo.
     * If MSMS was already set, then a re-start is sent prior to the
     * address.
     */
    if (!(Address & XIIC_TX_DYN_STOP_MASK))
    {

        XIic_DynSend7BitAddress(ip_ptr, Address,
                                XIIC_WRITE_OPERATION);
    }
    else
    {
        XIic_DynSendStartStopAddress(ip_ptr, Address,
                                     XIIC_WRITE_OPERATION);
    }

    /*
     * Wait for the bus to go busy.
     */
    StatusRegister = XIic_ReadReg(ip_ptr, XIIC_SR_REG_OFFSET);

    while ((StatusRegister & XIIC_SR_BUS_BUSY_MASK) !=
           XIIC_SR_BUS_BUSY_MASK)
    {
        StatusRegister = XIic_ReadReg(ip_ptr,
                                      XIIC_SR_REG_OFFSET);
        timeout_cnt++;
        if(timeout_cnt > timeout){
            //printf("debug 3\n");
            return 0;
        }
    }

    /*
     * Clear the latched interrupt status for the bus not busy bit which
     * must be done while the bus is busy.
     */
    XIic_ClearIisr(ip_ptr, XIIC_INTR_BNB_MASK);

    /*
     * Send the specified data to the device on the IIC bus specified by the
     * the address.
     */
    RemainingByteCount = DynSendData(ip_ptr, BufferPtr, ByteCount,
                                     Option, 10000);

    /*
     * The send is complete return the number of bytes that was sent.
     */
    return ByteCount - RemainingByteCount;
}

/******************************************************************************
 *
 * Send the specified buffer to the device that has been previously addressed
 * on the IIC bus. This function assumes that the 7 bit address has been sent.
 *
 * @param        ip_ptr contains the base address of the IIC Device.
 * @param        BufferPtr points to the data to be sent.
 * @param        ByteCount is the number of bytes to be sent.
 * @param        Option: XIIC_STOP = end with STOP condition, XIIC_REPEATED_START
 *               = don't end with STOP condition.
 *
 * @return       The number of bytes remaining to be sent.
 *
 * @note         This function does not take advantage of the transmit Fifo
 *               because it is designed for minimal code space and complexity.
 *
 ******************************************************************************/
static unsigned DynSendData(void *ip_ptr, u8 *BufferPtr,
                            u8 ByteCount, u8 Option, u32 timeout)
{
    u32 IntrStatus;
    u32 timeout_cnt = 0;

    while (ByteCount > 0)
    {
        /*
         * Wait for the transmit to be empty before sending any more
         * data by polling the interrupt status register.
         */
        while (1)
        {
            IntrStatus = XIic_ReadIisr(ip_ptr);
            if (IntrStatus & (XIIC_INTR_TX_ERROR_MASK |
                              XIIC_INTR_ARB_LOST_MASK |
                              XIIC_INTR_BNB_MASK))
            {
                /*
                 * Error condition (NACK or ARB Lost or BNB
                 * Error Has occurred. Clear the Control
                 * register to send a STOP condition on the Bus
                 * and return the number of bytes still to
                 * transmit.
                 */
                XIic_WriteReg(ip_ptr, XIIC_CR_REG_OFFSET,
                              0x03);
                XIic_WriteReg(ip_ptr, XIIC_CR_REG_OFFSET,
                              0x01);

                return ByteCount;
            }

            /*
             * Check for the transmit Fifo to become Empty.
             */
            if (IntrStatus & XIIC_INTR_TX_EMPTY_MASK)
            {
                break;
            }

            timeout_cnt++;
            if (timeout_cnt > timeout)
            {
                return ByteCount;
            }
        }

        /*
         * Send data to Tx Fifo. If a stop condition is specified and
         * the last byte is being sent, then set the dynamic stop bit.
         */
        if ((ByteCount == 1) && (Option == XIIC_STOP))
        {
            /*
             * The MSMS will be cleared automatically upon setting
             *  dynamic stop.
             */
            XIic_WriteReg(ip_ptr, XIIC_DTR_REG_OFFSET,
                          XIIC_TX_DYN_STOP_MASK | *BufferPtr++);
        }
        else
        {
            XIic_WriteReg(ip_ptr, XIIC_DTR_REG_OFFSET,
                          *BufferPtr++);
        }

        /*
         * Update the byte count to reflect the byte sent.
         */
        ByteCount--;
    }

    if (Option == XIIC_STOP)
    {
        /*
         * If the Option is to release the bus after transmission of
         * data, Wait for the bus to transition to not busy before
         * returning, the IIC device cannot be disabled until this
         * occurs.
         */
        timeout_cnt = 0;
        while (1)
        {
            if (XIic_ReadIisr(ip_ptr) & XIIC_INTR_BNB_MASK)
            {
                break;
            }
            timeout_cnt++;
            if (timeout_cnt > timeout)
            {
                break;
            }
        }
    }

    return ByteCount;
}

/******************************************************************************
 *
 * Initialize the IIC core for Dynamic Functionality.
 *
 * @param        ip_ptr contains the base address of the IIC Device.
 *
 * @return       XST_SUCCESS if Successful else XST_FAILURE.
 *
 * @note         None.
 *
 ******************************************************************************/
int XIic_DynInit(void *ip_ptr)
{
    u32 Status;

    /*
     * Reset IIC Core.
     */
    XIic_WriteReg(ip_ptr, XIIC_RESETR_OFFSET, XIIC_RESET_MASK);

    /*
     * Set receive Fifo depth to maximum (zero based).
     */
    XIic_WriteReg(ip_ptr, XIIC_RFD_REG_OFFSET,
                  IIC_RX_FIFO_DEPTH - 1);

    /*
     * Reset Tx Fifo.
     */
    XIic_WriteReg(ip_ptr, XIIC_CR_REG_OFFSET,
                  XIIC_CR_TX_FIFO_RESET_MASK);

    /*
     * Enable IIC Device, remove Tx Fifo reset & disable general call.
     */
    XIic_WriteReg(ip_ptr, XIIC_CR_REG_OFFSET,
                  XIIC_CR_ENABLE_DEVICE_MASK);

    /*
     * Read status register and verify IIC Device is in initial state. Only
     * the Tx Fifo and Rx Fifo empty bits should be set.
     */
    Status = XIic_ReadReg(ip_ptr, XIIC_SR_REG_OFFSET);
    if (Status == (XIIC_SR_RX_FIFO_EMPTY_MASK |
                   XIIC_SR_TX_FIFO_EMPTY_MASK))
    {
        return XST_SUCCESS;
    }

    return XST_FAILURE;
}

/*****************************************************************************
 *
 * This is a function which tells whether the I2C bus is busy or free.
 *
 * @param        BaseAddr is the base address of the I2C core to work on.
 *
 * @return
 *               - TRUE if the bus is busy.
 *               - FALSE if the bus is NOT busy.
 *
 * @note         None.
 *
 ******************************************************************************/
u32 XIic_CheckIsBusBusy(void *ip_ptr)
{
    u32 StatusReg;

    StatusReg = XIic_ReadReg(ip_ptr, XIIC_SR_REG_OFFSET);
    if (StatusReg & XIIC_SR_BUS_BUSY_MASK)
    {
       // printf("BUS BUZY \n");
        return TRUE;
    }
    else
    {
       // printf("BUS FREE \n ");
        return FALSE;
    }
}

/******************************************************************************/
/**
 * This function will wait until the I2C bus is free or timeout.
 *
 * @param        ip_ptr contains the base address of the I2C device.
 *
 * @return
 *               - XST_SUCCESS if the I2C bus was freed before the timeout.
 *               - XST_FAILURE otherwise.
 *
 * @note         None.
 *
 *******************************************************************************/
u32 XIic_WaitBusFree(void *ip_ptr)
{
    u32 BusyCount = 0;
    while (XIic_CheckIsBusBusy(ip_ptr))
    {
        if (BusyCount++ > 10000)
        {
            printf("BusyCount : %d \n", BusyCount);
            return XST_FAILURE;
        }
        printf("WAIT ...\n");
        usleep(100);
    }
  //  printf("debug 1 \n");

    return XST_SUCCESS;
}
/** @} */