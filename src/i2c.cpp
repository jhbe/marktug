#include "stm32f4xx.h"
#include "i2c.h"
#include "led.h"
#include "itime.h"
#include "debug.h"
#include "stdbulkusb.h"

#include <stdio.h>
#include <string.h>

/*
static uint32_t errors;
extern "C"
void I2C1_ER_IRQHandler(void) {
    if (I2C_GetITStatus(I2C1, I2C_IT_TIMEOUT) == SET) {
        errors |= I2C_IT_TIMEOUT;
        I2C_ClearFlag(I2C1, I2C_IT_TIMEOUT);
    }
    if (I2C_GetITStatus(I2C1, I2C_IT_PECERR) == SET) {
        errors |= I2C_IT_PECERR;
        I2C_ClearFlag(I2C1, I2C_IT_PECERR);
    }
    if (I2C_GetITStatus(I2C1, I2C_IT_OVR) == SET) {
        errors |= I2C_IT_OVR;
        I2C_ClearFlag(I2C1, I2C_IT_OVR);
    }
    if (I2C_GetITStatus(I2C1, I2C_IT_AF) == SET) {
        errors |= I2C_IT_AF;
        I2C_ClearFlag(I2C1, I2C_IT_AF);
    }
    if (I2C_GetITStatus(I2C1, I2C_IT_ARLO) == SET) {
        errors |= I2C_IT_ARLO;
        I2C_ClearFlag(I2C1, I2C_IT_ARLO);
    }
    if (I2C_GetITStatus(I2C1, I2C_IT_BERR) == SET) {
        errors |= I2C_IT_BERR;
        I2C_ClearFlag(I2C1, I2C_IT_BERR);
    }
    I2C_SoftwareResetCmd(I2C1, ENABLE);
}
*/

I2c::I2c(Time *time) {
    m_time = time;

    /*
     * Enable and reset the I2C peripheral clock on the APB1 bus.
     */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);

    /*
     * Enable the GPIOB peripheral clock on the AHB1 bus.
     */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

    /*
     * Assign PB6 (SCL) and PB7 (SDA) to the alternate function.
     */
    GPIO_InitTypeDef gi;
    gi.GPIO_Mode = GPIO_Mode_AF;
    gi.GPIO_OType = GPIO_OType_OD;
    gi.GPIO_PuPd = GPIO_PuPd_UP;
    gi.GPIO_Speed = GPIO_Low_Speed;
    gi.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_Init(GPIOB, &gi);

    /*
     * Connect PB6 (SCL) and PB7 (SDA) to I2C1.
     */
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_I2C1);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_I2C1);

    /*
     * Initiate I2C1.
     */
    I2C_InitTypeDef i2cInit;
    i2cInit.I2C_ClockSpeed = 100000;
    i2cInit.I2C_Mode = I2C_Mode_I2C;
    i2cInit.I2C_DutyCycle = I2C_DutyCycle_2;
    i2cInit.I2C_OwnAddress1 = 0; // Don't care in Master mode.
    i2cInit.I2C_Ack = I2C_Ack_Enable;
    i2cInit.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_Init(I2C1, &i2cInit);

    I2C_AcknowledgeConfig(I2C1, ENABLE);
    I2C_Cmd(I2C1, ENABLE);

    I2C_ITConfig(I2C1, I2C_IT_ERR, DISABLE);

    S5(false);
    S6(false);

}

bool I2c::WaitFor(uint32_t condition) {
    ErrorStatus status;
    for (uint64_t start = m_time->Now();
         ((m_time->Now() - start) < 2) && ((status = I2C_CheckEvent(I2C1, condition)) == ERROR);) {

    }
    return status == SUCCESS;
}

bool I2c::Write(uint8_t addr, uint8_t reg, uint8_t data) {

    bool done = false;
    do {
        I2C_GenerateSTART(I2C1, ENABLE);
        if (!WaitFor(I2C_EVENT_MASTER_MODE_SELECT)) {
            break;
        }

        I2C_Send7bitAddress(I2C1, addr << 1, I2C_Direction_Transmitter);
        if (!WaitFor(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)) {
            break;
        }

        I2C_SendData(I2C1, reg);
        if (!WaitFor(I2C_EVENT_MASTER_BYTE_TRANSMITTED)) {
            break;
        }

        I2C_SendData(I2C1, data);
        if (!WaitFor(I2C_EVENT_MASTER_BYTE_TRANSMITTED)) {
            break;
        }

        done = true;
    } while (false);

    I2C_GenerateSTOP(I2C1, ENABLE);

    return done;
}

bool I2c::Read(uint8_t addr, uint8_t reg, uint8_t &data) {
    I2C_AcknowledgeConfig(I2C1, ENABLE);

    bool done = false;
    do {
        I2C_GenerateSTART(I2C1, ENABLE);
        if (!WaitFor(I2C_EVENT_MASTER_MODE_SELECT)) {
            break;
        }

        I2C_Send7bitAddress(I2C1, addr << 1, I2C_Direction_Transmitter);
        if (!WaitFor(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)) {
            break;
        }

        I2C_SendData(I2C1, reg);
        if (!WaitFor(I2C_EVENT_MASTER_BYTE_TRANSMITTED)) {
            break;
        }

        I2C_GenerateSTOP(I2C1, ENABLE);

        I2C_GenerateSTART(I2C1, ENABLE);
        if (!WaitFor(I2C_EVENT_MASTER_MODE_SELECT)) {
            break;
        }

        I2C_Send7bitAddress(I2C1, addr << 1, I2C_Direction_Receiver);
        if (!WaitFor(I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED)) {
            break;
        }

        I2C_AcknowledgeConfig(I2C1, DISABLE);

        if (!WaitFor(I2C_EVENT_MASTER_BYTE_RECEIVED)) {
            continue;
        }

        data = I2C_ReceiveData(I2C1);

        done = true;
    } while (false);

    I2C_GenerateSTOP(I2C1, ENABLE);

    I2C_AcknowledgeConfig(I2C1, ENABLE);

/*    if (!done) {
        I2C_SoftwareResetCmd(I2C1, ENABLE);
        I2C_SoftwareResetCmd(I2C1, DISABLE);
    }
*/    return done;
}
/*
bool I2c::ReadBytes(uint8_t addr, uint8_t reg, uint8_t *data, uint8_t length) {
    I2C_AcknowledgeConfig(I2C1, ENABLE);
    I2C_NACKPositionConfig(I2C1, I2C_NACKPosition_Current);

    bool done = false;
    do {
        I2C_GenerateSTART(I2C1, ENABLE);
        if (!WaitFor(I2C_EVENT_MASTER_MODE_SELECT)) {
            break;
        }

        I2C_Send7bitAddress(I2C1, addr << 1, I2C_Direction_Transmitter);
        if (!WaitFor(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)) {
            break;
        }

        I2C_SendData(I2C1, reg);
        if (!WaitFor(I2C_EVENT_MASTER_BYTE_TRANSMITTED)) {
            break;
        }

        I2C_GenerateSTOP(I2C1, ENABLE);

        I2C_GenerateSTART(I2C1, ENABLE);
        if (!WaitFor(I2C_EVENT_MASTER_MODE_SELECT)) {
            break;
        }

        I2C_Send7bitAddress(I2C1, addr << 1, I2C_Direction_Receiver);
        if (!WaitFor(I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED)) {
            break;
        }

        for (int i = 0; i < length; i++) {
            I2C_NACKPositionConfig(I2C1, I2C_NACKPosition_Next);

            if (i == length - 1) {
                I2C_AcknowledgeConfig(I2C1, DISABLE);
            }

            if (!WaitFor(I2C_EVENT_MASTER_BYTE_RECEIVED)) {
                continue;
            }

            data[i] = I2C_ReceiveData(I2C1);
        }
        done = true;
    } while (false);

    if (!done) {
        S6 (true);
    }

    I2C_GenerateSTOP(I2C1, ENABLE);

    I2C_AcknowledgeConfig(I2C1, ENABLE);
    I2C_NACKPositionConfig(I2C1, I2C_NACKPosition_Current);

    return done;
}
*/

bool I2c::ReadBytes(uint8_t addr, uint8_t reg, uint8_t *data, uint8_t length) {
    S5 (true);
    S6 (true);
    I2C1->CR1 |= I2C_CR1_ACK;   // Enable ACK
    I2C1->CR1 &= ~I2C_CR1_POS;  // NACK position current

    I2C1->CR1 |= I2C_CR1_START; // START

    while (1) {
        uint16_t sr1 = I2C1->SR1;
        uint16_t sr2 = I2C1->SR2;
        if ((sr1 & I2C_SR1_SB) &&
            (sr2 & I2C_SR2_BUSY) && (sr2 & I2C_SR2_MSL)) {
            break;
        }
    }

    I2C1->DR = (addr << 1);

    while (1) {
        uint16_t sr1 = I2C1->SR1;
        uint16_t sr2 = I2C1->SR2;
        if ((sr1 & I2C_SR1_TXE) && (sr1 & I2C_SR1_ADDR) &&
            (sr2 & I2C_SR2_BUSY) && (sr2 & I2C_SR2_MSL) && (sr2 & I2C_SR2_TRA)) {
            break;
        }
    }

    I2C1->DR = reg;

    while (1) {
        uint16_t sr1 = I2C1->SR1;
        uint16_t sr2 = I2C1->SR2;
        if (sr1 & I2C_SR1_BTF) {
            break;
        }
    }

    I2C1->CR1 |= I2C_CR1_STOP;

    while (1) {
        uint16_t sr1 = I2C1->SR1;
        uint16_t sr2 = I2C1->SR2;
        if ((sr1 & I2C_SR1_STOPF) == 0) {
            break;
        }
    }

    // READ
    S6(false);

    I2C1->CR1 |= I2C_CR1_START; // START

    while (1) {
        uint16_t sr1 = I2C1->SR1;
        uint16_t sr2 = I2C1->SR2;
        if ((sr1 & I2C_SR1_SB) &&
            (sr2 & I2C_SR2_BUSY) && (sr2 & I2C_SR2_MSL)) {
            break;
        }
    }

    I2C1->DR = (addr << 1) | 0x01;

    while (1) {
        uint16_t sr1 = I2C1->SR1;
        uint16_t sr2 = I2C1->SR2;
        if (sr1 & I2C_SR1_ADDR) {
            break;
        }
    }

    I2C1->SR1; // Clear ADDR bit;
    I2C1->SR2;

    while (length > 3) {
        while (1) {
            uint16_t sr1 = I2C1->SR1;
            uint16_t sr2 = I2C1->SR2;
            if (sr1 & I2C_SR1_BTF) {
                break;
            }
        }

        *data++ = (uint8_t)I2C1->DR;
        length--;
    }

    while (1) {
        uint16_t sr1 = I2C1->SR1;
        uint16_t sr2 = I2C1->SR2;
        if (sr1 & I2C_SR1_BTF) {
            break;
        }
    }

    I2C1->CR1 &= (uint16_t)~((uint16_t)I2C_CR1_ACK);

    __disable_irq();
    *data++ = (uint8_t)I2C1->DR;

    I2C1->CR1 |= I2C_CR1_STOP;
    __enable_irq();

    *data++ = (uint8_t)I2C1->DR;

    while (1) {
        uint16_t sr1 = I2C1->SR1;
        uint16_t sr2 = I2C1->SR2;
        if ((sr1 & I2C_SR1_RXNE) &&
            (sr2 & I2C_SR2_BUSY) && (sr2 & I2C_SR2_MSL)) {
            break;
        }
    }

    *data++ = (uint8_t)I2C1->DR;

    while (1) {
        uint16_t sr1 = I2C1->SR1;
        uint16_t sr2 = I2C1->SR2;
        if ((sr1 & I2C_SR1_STOPF) == 0) {
            break;
        }
    }

    I2C1->CR1 |= I2C_CR1_ACK;   // Enable ACK

    S5(false);

    return true;
}
