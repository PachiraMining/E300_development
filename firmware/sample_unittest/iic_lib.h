#ifndef IIC_2_H /* prevent circular inclusions */
#define IIC_2_H /* by using protection macros */

#ifdef __cplusplus
extern "C"
{
#endif

/***************************** Include Files ********************************/

// #include "xil_types.h"
// #include "xil_assert.h"
// #include "xstatus.h"
// #include "xil_io.h"
#include <stdio.h>

    typedef uint8_t u8;
    typedef uint16_t u16;
    typedef uint32_t u32;

#define XST_SUCCESS 0L
#define XST_FAILURE 1L

#ifndef TRUE
#define TRUE 1U
#endif

#ifndef FALSE
#define FALSE 0U
#endif

#ifndef NULL
#define NULL 0U
#endif
/************************** Constant Definitions ****************************/

/** @name Register Map
 *
 * Register offsets for the XIic device.
 * @{
 */
#define XIIC_DGIER_OFFSET 0x1C    /**< Global Interrupt Enable Register */
#define XIIC_IISR_OFFSET 0x20     /**< Interrupt Status Register */
#define XIIC_IIER_OFFSET 0x28     /**< Interrupt Enable Register */
#define XIIC_RESETR_OFFSET 0x40   /**< Reset Register */
#define XIIC_CR_REG_OFFSET 0x100  /**< Control Register */
#define XIIC_SR_REG_OFFSET 0x104  /**< Status Register */
#define XIIC_DTR_REG_OFFSET 0x108 /**< Data Tx Register */
#define XIIC_DRR_REG_OFFSET 0x10C /**< Data Rx Register */
#define XIIC_ADR_REG_OFFSET 0x110 /**< Address Register */
#define XIIC_TFO_REG_OFFSET 0x114 /**< Tx FIFO Occupancy */
#define XIIC_RFO_REG_OFFSET 0x118 /**< Rx FIFO Occupancy */
#define XIIC_TBA_REG_OFFSET 0x11C /**< 10 Bit Address reg */
#define XIIC_RFD_REG_OFFSET 0x120 /**< Rx FIFO Depth reg */
#define XIIC_GPO_REG_OFFSET 0x124 /**< Output Register */
/* @} */

/**
 * @name Device Global Interrupt Enable Register masks (CR) mask(s)
 * @{
 */
#define XIIC_GINTR_ENABLE_MASK 0x80000000 /**< Global Interrupt Enable Mask */
/* @} */

/** @name IIC Device Interrupt Status/Enable (INTR) Register Masks
 *
 * <b> Interrupt Status Register (IISR) </b>
 *
 * This register holds the interrupt status flags for the Spi device.
 *
 * <b> Interrupt Enable Register (IIER) </b>
 *
 * This register is used to enable interrupt sources for the IIC device.
 * Writing a '1' to a bit in this register enables the corresponding Interrupt.
 * Writing a '0' to a bit in this register disables the corresponding Interrupt.
 *
 * IISR/IIER registers have the same bit definitions and are only defined once.
 * @{
 */
#define XIIC_INTR_ARB_LOST_MASK 0x00000001 /**< 1 = Arbitration lost */
#define XIIC_INTR_TX_ERROR_MASK 0x00000002 /**< 1 = Tx error/msg complete */
#define XIIC_INTR_TX_EMPTY_MASK 0x00000004 /**< 1 = Tx FIFO/reg empty */
#define XIIC_INTR_RX_FULL_MASK 0x00000008  /**< 1 = Rx FIFO/reg=OCY level */
#define XIIC_INTR_BNB_MASK 0x00000010      /**< 1 = Bus not busy */
#define XIIC_INTR_AAS_MASK 0x00000020      /**< 1 = When addr as slave */
#define XIIC_INTR_NAAS_MASK 0x00000040     /**< 1 = Not addr as slave */
#define XIIC_INTR_TX_HALF_MASK 0x00000080  /**< 1 = Tx FIFO half empty */

/**
 * All Tx interrupts commonly used.
 */
#define XIIC_TX_INTERRUPTS (XIIC_INTR_TX_ERROR_MASK | \
                            XIIC_INTR_TX_EMPTY_MASK | \
                            XIIC_INTR_TX_HALF_MASK)

/**
 * All interrupts commonly used
 */
#define XIIC_TX_RX_INTERRUPTS (XIIC_INTR_RX_FULL_MASK | XIIC_TX_INTERRUPTS)

/* @} */

/**
 * @name Reset Register mask
 * @{
 */
#define XIIC_RESET_MASK 0x0000000A /**< RESET Mask  */
/* @} */

/**
 * @name Control Register masks (CR) mask(s)
 * @{
 */
#define XIIC_CR_ENABLE_DEVICE_MASK 0x00000001  /**< Device enable = 1 */
#define XIIC_CR_TX_FIFO_RESET_MASK 0x00000002  /**< Transmit FIFO reset=1 */
#define XIIC_CR_MSMS_MASK 0x00000004           /**< Master starts Txing=1 */
#define XIIC_CR_DIR_IS_TX_MASK 0x00000008      /**< Dir of Tx. Txing=1 */
#define XIIC_CR_NO_ACK_MASK 0x00000010         /**< Tx Ack. NO ack = 1 */
#define XIIC_CR_REPEATED_START_MASK 0x00000020 /**< Repeated start = 1 */
#define XIIC_CR_GENERAL_CALL_MASK 0x00000040   /**< Gen Call enabled = 1 */
/* @} */

/**
 * @name Status Register masks (SR) mask(s)
 * @{
 */
#define XIIC_SR_GEN_CALL_MASK 0x00000001         /**< 1 = A Master issued \
                                                  * a GC */
#define XIIC_SR_ADDR_AS_SLAVE_MASK 0x00000002    /**< 1 = When addressed as \
                                                  * slave */
#define XIIC_SR_BUS_BUSY_MASK 0x00000004         /**< 1 = Bus is busy */
#define XIIC_SR_MSTR_RDING_SLAVE_MASK 0x00000008 /**< 1 = Dir: Master <-- \
                                                  * slave */
#define XIIC_SR_TX_FIFO_FULL_MASK 0x00000010     /**< 1 = Tx FIFO full */
#define XIIC_SR_RX_FIFO_FULL_MASK 0x00000020     /**< 1 = Rx FIFO full */
#define XIIC_SR_RX_FIFO_EMPTY_MASK 0x00000040    /**< 1 = Rx FIFO empty */
#define XIIC_SR_TX_FIFO_EMPTY_MASK 0x00000080    /**< 1 = Tx FIFO empty */
/* @} */

/**
 * @name Data Tx Register (DTR) mask(s)
 * @{
 */
#define XIIC_TX_DYN_START_MASK 0x00000100 /**< 1 = Set dynamic start */
#define XIIC_TX_DYN_STOP_MASK 0x00000200  /**< 1 = Set dynamic stop */
#define IIC_TX_FIFO_DEPTH 16              /**< Tx fifo capacity */
/* @} */

/**
 * @name Data Rx Register (DRR) mask(s)
 * @{
 */
#define IIC_RX_FIFO_DEPTH 16 /**< Rx fifo capacity */
    /* @} */

#define XIIC_TX_ADDR_SENT 0x00
#define XIIC_TX_ADDR_MSTR_RECV_MASK 0x02

/**
 * The following constants are used to specify whether to do
 * Read or a Write operation on IIC bus.
 */
#define XIIC_READ_OPERATION 1  /**< Read operation on the IIC bus */
#define XIIC_WRITE_OPERATION 0 /**< Write operation on the IIC bus */

/**
 * The following constants are used with the transmit FIFO fill function to
 * specify the role which the IIC device is acting as, a master or a slave.
 */
#define XIIC_MASTER_ROLE 1 /**< Master on the IIC bus */
#define XIIC_SLAVE_ROLE 0  /**< Slave on the IIC bus */

/**
 * The following constants are used with Transmit Function (XIic_Send) to
 * specify whether to STOP after the current transfer of data or own the bus
 * with a Repeated start.
 */
#define XIIC_STOP 0x00           /**< Send a stop on the IIC bus after \
                                  * the current data transfer */
#define XIIC_REPEATED_START 0x01 /**< Donot Send a stop on the IIC bus after \
                                  * the current data transfer */

/***************** Macros (Inline Functions) Definitions *********************/

// #define XIic_In32    Xil_In32
// #define XIic_Out32   Xil_Out32

/****************************************************************************/
/**
 *
 * Read from the specified IIC device register.
 *
 * @param        ip_ptr is the base address of the device.
 * @param        RegOffset is the offset from the 1st register of the device to
 *               select the specific register.
 *
 * @return       The value read from the register.
 *
 * @note         C-Style signature:
 *               u32 XIic_ReadReg(u32 ip_ptr, u32 RegOffset);
 *
 *               This macro does not do any checking to ensure that the
 *               register exists if the register may be excluded due to
 *               parameterization, such as the GPO Register.
 *
 ******************************************************************************/
#define XIic_ReadReg(ip_ptr, RegOffset) *((unsigned *)(((u_int8_t *)ip_ptr) + RegOffset))
    // XIic_In32((ip_ptr) + (RegOffset))

/***************************************************************************/
/**
 *
 * Write to the specified IIC device register.
 *
 * @param        ip_ptr is the base address of the device.
 * @param        RegOffset is the offset from the 1st register of the
 *               device to select the specific register.
 * @param        RegisterValue is the value to be written to the register.
 *
 * @return       None.
 *
 * @note         C-Style signature:
 *               void XIic_WriteReg(u32 ip_ptr, u32 RegOffset,
 *                                       u32 RegisterValue);
 *               This macro does not do any checking to ensure that the
 *               register exists if the register may be excluded due to
 *               parameterization, such as the GPO Register.
 *
 ******************************************************************************/
#define XIic_WriteReg(ip_ptr, RegOffset, RegisterValue) *((unsigned *)(((u_int8_t *)ip_ptr) + RegOffset)) = RegisterValue
    // #define XIic_WriteReg(ip_ptr, RegOffset, RegisterValue) \
// {                                            \
//      *((unsigned *)(((u_int8_t *)ip_ptr) + RegOffset)) = RegisterValue; \
//      printf("send %02x \t", RegisterValue); \

    // }

    // #define XIic_WriteReg(ip_ptr, RegOffset, RegisterValue)              \
// {                                                                            \
//      *((unsigned *)(((u_int8_t *)ip_ptr) + RegOffset)) = RegisterValue;      \
//      printf("send %02x \t", RegisterValue);                  \
// }

    // #define XIic_Send7BitAddress(ip_ptr, SlaveAddress, Operation)        \
// {                                                                    \
//      u8 LocalAddr = (u8)(SlaveAddress << 1);                         \
//      LocalAddr = (LocalAddr & 0xFE) | (Operation);                   \
//      XIic_WriteReg(ip_ptr, XIIC_DTR_REG_OFFSET, LocalAddr);  \
// }

    // XIic_Out32((ip_ptr) + (RegOffset), (RegisterValue))

/******************************************************************************/
/**
 *
 * This macro disables all interrupts for the device by writing to the Global
 * interrupt enable register.
 *
 * @param        ip_ptr is the base address of the IIC device.
 *
 * @return       None.
 *
 * @note         C-Style signature:
 *               void XIic_IntrGlobalDisable(u32 ip_ptr);
 *
 ******************************************************************************/
#define XIic_IntrGlobalDisable(ip_ptr) \
    XIic_WriteReg((ip_ptr), XIIC_DGIER_OFFSET, 0)

/******************************************************************************/
/**
 *
 * This macro writes to the global interrupt enable register to enable
 * interrupts from the device. This function does not enable individual
 * interrupts as the Interrupt Enable Register must be set appropriately.
 *
 * @param        ip_ptr is the base address of the IIC device.
 *
 * @return       None.
 *
 * @note         C-Style signature:
 *               void XIic_IntrGlobalEnable(u32 ip_ptr);
 *
 ******************************************************************************/
#define XIic_IntrGlobalEnable(ip_ptr)          \
    XIic_WriteReg((ip_ptr), XIIC_DGIER_OFFSET, \
                  XIIC_GINTR_ENABLE_MASK)

/******************************************************************************/
/**
 *
 * This function determines if interrupts are enabled at the global level by
 * reading the global interrupt register.
 *
 * @param        ip_ptr is the base address of the IIC device.
 *
 * @return
 *               - TRUE if the global interrupt is enabled.
 *               - FALSE if global interrupt is disabled.
 *
 * @note         C-Style signature:
 *               int XIic_IsIntrGlobalEnabled(u32 ip_ptr);
 *
 ******************************************************************************/
#define XIic_IsIntrGlobalEnabled(ip_ptr)          \
    (XIic_ReadReg((ip_ptr), XIIC_DGIER_OFFSET) == \
     XIIC_GINTR_ENABLE_MASK)

/******************************************************************************/
/**
 *
 * This function sets the Interrupt status register to the specified value.
 *
 * This register implements a toggle on write functionality. The interrupt is
 * cleared by writing to this register with the bits to be cleared set to a one
 * and all others to zero. Setting a bit which is zero within this register
 * causes an interrupt to be generated.
 *
 * This function writes only the specified value to the register such that
 * some status bits may be set and others cleared.  It is the caller's
 * responsibility to get the value of the register prior to setting the value
 * to prevent an destructive behavior.
 *
 * @param        ip_ptr is the base address of the IIC device.
 * @param        Status is the value to be written to the Interrupt
 *               status register.
 *
 * @return       None.
 *
 * @note         C-Style signature:
 *               void XIic_WriteIisr(u32 ip_ptr, u32 Status);
 *
 ******************************************************************************/
#define XIic_WriteIisr(ip_ptr, Status) \
    XIic_WriteReg((ip_ptr), XIIC_IISR_OFFSET, (Status))

/******************************************************************************/
/**
 *
 * This function gets the contents of the Interrupt Status Register.
 * This register indicates the status of interrupt sources for the device.
 * The status is independent of whether interrupts are enabled such
 * that the status register may also be polled when interrupts are not enabled.
 *
 * @param        ip_ptr is the base address of the IIC device.
 *
 * @return       The value read from the Interrupt Status Register.
 *
 * @note         C-Style signature:
 *               u32 XIic_ReadIisr(u32 ip_ptr);
 *
 ******************************************************************************/
#define XIic_ReadIisr(ip_ptr) \
    XIic_ReadReg((ip_ptr), XIIC_IISR_OFFSET)

/******************************************************************************/
/**
 *
 * This function sets the contents of the Interrupt Enable Register.
 *
 * This function writes only the specified value to the register such that
 * some interrupt sources may be enabled and others disabled.  It is the
 * caller's responsibility to get the value of the interrupt enable register
 * prior to setting the value to prevent a destructive behavior.
 *
 * @param        ip_ptr is the base address of the IIC device.
 * @param        Enable is the value to be written to the Interrupt Enable
 *               Register. Bit positions of 1 will be enabled. Bit positions of 0
 *               will be disabled.
 *
 * @return       None
 *
 * @note         C-Style signature:
 *               void XIic_WriteIier(u32 ip_ptr, u32 Enable);
 *
 ******************************************************************************/
#define XIic_WriteIier(ip_ptr, Enable) \
    XIic_WriteReg((ip_ptr), XIIC_IIER_OFFSET, (Enable))

/******************************************************************************/
/**
 *
 *
 * This function gets the Interrupt Enable Register contents.
 *
 * @param        ip_ptr is the base address of the IIC device.
 *
 * @return       The contents read from the Interrupt Enable Register.
 *               Bit positions of 1 indicate that the corresponding interrupt
 *               is enabled. Bit positions of 0 indicate that the corresponding
 *               interrupt is disabled.
 *
 * @note         C-Style signature:
 *               u32 XIic_ReadIier(u32 ip_ptr)
 *
 ******************************************************************************/
#define XIic_ReadIier(ip_ptr) \
    XIic_ReadReg((ip_ptr), XIIC_IIER_OFFSET)

/******************************************************************************/
/**
 *
 * This macro clears the specified interrupt in the Interrupt status
 * register.  It is non-destructive in that the register is read and only the
 * interrupt specified is cleared.  Clearing an interrupt acknowledges it.
 *
 * @param        ip_ptr is the base address of the IIC device.
 * @param        InterruptMask is the bit mask of the interrupts to be cleared.
 *
 * @return       None.
 *
 * @note         C-Style signature:
 *               void XIic_ClearIisr(u32 ip_ptr, u32 InterruptMask);
 *
 ******************************************************************************/
#define XIic_ClearIisr(ip_ptr, InterruptMask) \
    XIic_WriteIisr((ip_ptr),                  \
                   XIic_ReadIisr(ip_ptr) & (InterruptMask))

/******************************************************************************/
/**
 *
 * This macro sends the address for a 7 bit address during both read and write
 * operations. It takes care of the details to format the address correctly.
 * This macro is designed to be called internally to the drivers.
 *
 * @param        ip_ptr is the base address of the IIC Device.
 * @param        SlaveAddress is the address of the slave to send to.
 * @param        Operation indicates XIIC_READ_OPERATION or XIIC_WRITE_OPERATION
 *
 * @return       None.
 *
 * @note         C-Style signature:
 *               void XIic_Send7BitAddress(u32 ip_ptr, u8 SlaveAddress,
 *                                               u8 Operation);
 *
 ******************************************************************************/
#define XIic_Send7BitAddress(ip_ptr, SlaveAddress, Operation)  \
    {                                                          \
        u8 LocalAddr = (u8)(SlaveAddress << 1);                \
        LocalAddr = (LocalAddr & 0xFE) | (Operation);          \
        XIic_WriteReg(ip_ptr, XIIC_DTR_REG_OFFSET, LocalAddr); \
    }

/******************************************************************************/
/**
 *
 * This macro sends the address for a 7 bit address during both read and write
 * operations. It takes care of the details to format the address correctly.
 * This macro is designed to be called internally to the drivers for Dynamic
 * controller functionality.
 *
 * @param        ip_ptr is the base address of the IIC Device.
 * @param        SlaveAddress is the address of the slave to send to.
 * @param        Operation indicates XIIC_READ_OPERATION or XIIC_WRITE_OPERATION.
 *
 * @return       None.
 *
 * @note         C-Style signature:
 *               void XIic_DynSend7BitAddress(u32 ip_ptr,
 *                               u8 SlaveAddress, u8 Operation);
 *
 ******************************************************************************/
#define XIic_DynSend7BitAddress(ip_ptr, SlaveAddress, Operation) \
    {                                                            \
        u8 LocalAddr = (u8)(SlaveAddress << 1);                  \
        LocalAddr = (LocalAddr & 0xFE) | (Operation);            \
        XIic_WriteReg(ip_ptr, XIIC_DTR_REG_OFFSET,               \
                      XIIC_TX_DYN_START_MASK | LocalAddr);       \
    }

/******************************************************************************/
/**
 *
 * This macro sends the address, start and stop for a 7 bit address during both
 * write operations. It takes care of the details to format the address
 * correctly. This macro is designed to be called internally to the drivers.
 *
 * @param        ip_ptr is the base address of the IIC Device.
 * @param        SlaveAddress is the address of the slave to send to.
 * @param        Operation indicates XIIC_WRITE_OPERATION.
 *
 * @return       None.
 *
 * @note         C-Style signature:
 *               void XIic_DynSendStartStopAddress(u32 ip_ptr,
 *                                                       u8 SlaveAddress,
 *                                                       u8 Operation);
 *
 ******************************************************************************/
#define XIic_DynSendStartStopAddress(ip_ptr, SlaveAddress, Operation)  \
    {                                                                  \
        u8 LocalAddr = (u8)(SlaveAddress << 1);                        \
        LocalAddr = (LocalAddr & 0xFE) | (Operation);                  \
        XIic_WriteReg(ip_ptr, XIIC_DTR_REG_OFFSET,                     \
                      XIIC_TX_DYN_START_MASK | XIIC_TX_DYN_STOP_MASK | \
                          LocalAddr);                                  \
    }

/******************************************************************************/
/**
 *
 * This macro sends a stop condition on IIC bus for Dynamic logic.
 *
 * @param        ip_ptr is the base address of the IIC Device.
 * @param        ByteCount is the number of Rx bytes received before the master.
 *               doesn't respond with ACK.
 *
 * @return       None.
 *
 * @note         C-Style signature:
 *               void XIic_DynSendStop(u32 ip_ptr, u32 ByteCount);
 *
 ******************************************************************************/
#define XIic_DynSendStop(ip_ptr, ByteCount)               \
    {                                                     \
        XIic_WriteReg(ip_ptr, XIIC_DTR_REG_OFFSET,        \
                      XIIC_TX_DYN_STOP_MASK | ByteCount); \
    }

    /************************** Function Prototypes *****************************/

    unsigned XIic_Recv(void *ip_ptr, u8 Address,
                       u8 *BufferPtr, unsigned ByteCount, u8 Option, u32 timeout);

    unsigned XIic_Send(void *ip_ptr, u8 Address,
                       u8 *BufferPtr, unsigned ByteCount, u8 Option, u32 timeout);

    unsigned XIic_DynRecv(void *ip_ptr, u8 Address, u8 *BufferPtr, u8 ByteCount, u32 timeout);

    unsigned XIic_DynSend(void *ip_ptr, u16 Address, u8 *BufferPtr,
                          u8 ByteCount, u8 Option, u32 timeout);

    int XIic_DynInit(void *ip_ptr);

    u32 XIic_CheckIsBusBusy(void *ip_ptr);

    u32 XIic_WaitBusFree(void *ip_ptr);

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
       /** @} */