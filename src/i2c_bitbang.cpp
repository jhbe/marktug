#include "stm32f4xx.h"
#include "i2c.h"
#include "led.h"
#include "itime.h"
#include "debug.h"
#include "stdbulkusb.h"

#include <stdio.h>
#include <string.h>

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
     * Configure P6 and P7 as open drain outputs with pull-up resistors.
     */
    GPIO_InitTypeDef gi;
    gi.GPIO_Mode  = GPIO_Mode_OUT;
    gi.GPIO_OType = GPIO_OType_OD;
    gi.GPIO_PuPd  = GPIO_PuPd_UP;
    gi.GPIO_Speed = GPIO_Low_Speed;
    gi.GPIO_Pin   = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_Init(GPIOB, &gi);
}

void Delay() {
    volatile int i = 50;
    while (i) {
        i--;
    }
}

#define CL_H  (GPIOB->BSRR = GPIO_Pin_6)
#define CL_L  (GPIOB->BSRR = GPIO_Pin_6 << 16)
#define DA_H  (GPIOB->BSRR = GPIO_Pin_7)
#define DA_L  (GPIOB->BSRR = GPIO_Pin_7 << 16)
#define DA_IN (GPIOB->IDR & GPIO_Pin_7)

#define DELAY Delay()

void Start(void) {
    CL_H;
    DA_H;
    DELAY;
    DA_L;
    DELAY;
}

void Stop(void) {
    CL_L;
    DA_L;
    DELAY;
    CL_H;
    DELAY;
    DA_H;
    DELAY;
}

void Ack(void) {
    CL_L;
    DELAY;
    DA_L;
    DELAY;
    CL_H;
    DELAY;
    CL_L;
    DELAY;
}

void NoAck(void) {
    CL_L;
    DELAY;
    DA_H;
    DELAY;
    CL_H;
    DELAY;
    CL_L;
    DELAY;
}

bool WaitAck(void) {
    bool acked = false;
    CL_L;
    DELAY;
    DA_H;
    DELAY;
    CL_H;
    DELAY;
    acked = DA_IN;
    CL_L;
    return acked;
}

void WriteByte(uint8_t byte) {
    for (int i = 0; i < 8; i++) {
        CL_L;
        DELAY;
        if (byte & 0x80) {
            DA_H;
        } else {
            DA_L;
        }
        byte <<= 1;
        DELAY;
        CL_H;
        DELAY;
    }
    CL_L;
}

uint8_t ReadByte(void) {
    uint8_t byte = 0;
    DA_H;

    for (int i = 0; i < 8; i++) {
        byte <<= 1;
        CL_L;
        DELAY;
        CL_H;
        DELAY;
        if (DA_IN) {
            byte |= 0x01;
        }
    }
    CL_L;
    return byte;
}

bool I2c::Write(uint8_t addr, uint8_t reg, uint8_t data) {
    Start();
    WriteByte(addr << 1);
    WaitAck();
    WriteByte(reg);
    WaitAck();
    WriteByte(data);
    WaitAck();
    Stop();

    return true;
}

bool I2c::ReadBytes(uint8_t addr, uint8_t reg, uint8_t *data, uint8_t length) {
    Start();
    WriteByte(addr << 1);
    WaitAck();
    WriteByte(reg);
    WaitAck();
    Stop();
    Start();
    WriteByte((addr << 1) | 0x01);
    WaitAck();
    for (int i = 0; i < length; i++) {
        data[i] = ReadByte();
        if (i == length - 1) {
            NoAck();
        } else {
            Ack();
        }
    }
    Stop();

    return true;
}
