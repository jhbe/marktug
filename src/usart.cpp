#include <stm32f4xx.h>
#include "usart.h"

Usart::Usart(USART_TypeDef *usart, uint32_t baudRate, bool fullDuplex, GPIO_TypeDef *gpio, uint16_t txPin,
             uint16_t rxPin, uint8_t txPinSource, uint8_t rxPinSource) {
    m_usart = usart;

    /*
     * Enable PORT A.
     */
    if (gpio == GPIOA) {
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    } else {
        assert_param(false);
    }

    /*
     * Enable the USART peripheral clock on the correct bus.
     */
    if (m_usart == USART1) {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    } else if (m_usart == UART4) {
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);
    } else {
        assert_param(false);
    }

    GPIO_InitTypeDef gis;
    gis.GPIO_Mode = GPIO_Mode_AF;
    gis.GPIO_OType = GPIO_OType_OD;
    gis.GPIO_Speed = GPIO_Speed_100MHz;
    gis.GPIO_PuPd = GPIO_PuPd_UP;
    if (fullDuplex) {
        gis.GPIO_Pin = txPin | rxPin;
    } else {
        gis.GPIO_Pin = txPin;
    }
    GPIO_Init(gpio, &gis);

    if (m_usart == USART1) {
        GPIO_PinAFConfig(gpio, txPinSource, GPIO_AF_USART1);
        if (fullDuplex) {
            GPIO_PinAFConfig(gpio, rxPinSource, GPIO_AF_USART1);
        }
    } else if (m_usart == UART4) {
        GPIO_PinAFConfig(gpio, txPinSource, GPIO_AF_UART4);
        if (fullDuplex) {
            GPIO_PinAFConfig(gpio, rxPinSource, GPIO_AF_UART4);
        }
    } else {
        assert_param(false);
    }

    NVIC_InitTypeDef ii;
    if (m_usart == USART1) {
        ii.NVIC_IRQChannel = USART1_IRQn;
    } else if (m_usart == UART4) {
        ii.NVIC_IRQChannel = UART4_IRQn;
    } else {
        assert_param(false);
    }
    ii.NVIC_IRQChannelCmd = ENABLE;
    ii.NVIC_IRQChannelPreemptionPriority = 0;
    ii.NVIC_IRQChannelSubPriority = 1;
    NVIC_Init(&ii);

    USART_InitTypeDef ui;
    ui.USART_BaudRate = baudRate;
    ui.USART_WordLength = USART_WordLength_8b;
    ui.USART_StopBits = USART_StopBits_1;
    ui.USART_Parity = USART_Parity_No;
    ui.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    ui.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_Init(m_usart, &ui);

    USART_Cmd(m_usart, ENABLE);
    USART_HalfDuplexCmd(m_usart, fullDuplex ? DISABLE : ENABLE);

    USART_ITConfig(m_usart, USART_IT_RXNE, ENABLE);
    USART_ClearITPendingBit(m_usart, USART_IT_RXNE);
}

void Usart::OnInterrupt() {
    if (USART_GetITStatus(m_usart, USART_IT_ORE) == SET) {
        USART_ClearITPendingBit(m_usart, USART_IT_ORE);
        TryToReadOneByte();
    }
    if (USART_GetITStatus(m_usart, USART_IT_TXE) == SET) {
        TryToSendOneByte();
    }
    if (USART_GetITStatus(m_usart, USART_IT_RXNE) == SET) {
        TryToReadOneByte();
    }
}

bool Usart::Read(uint8_t &byte) {
    return m_rx_fifo.Get(byte);
}

void Usart::Write(uint8_t *buffer, uint16_t length) {
    for (int i = 0; i < length; i++) {
        m_tx_fifo.Put(buffer[i]);
    }
    TryToSendOneByte();
}

void Usart::TryToSendOneByte(void) {
    if (USART_GetFlagStatus(m_usart, USART_FLAG_TXE) == SET) {
        uint8_t byte;
        if (m_tx_fifo.Get(byte)) {
            USART_SendData(m_usart, byte);
            USART_ITConfig(m_usart, USART_IT_TXE, ENABLE);
        } else {
            USART_ITConfig(m_usart, USART_IT_TXE, DISABLE);
        }
    }
}

void Usart::TryToReadOneByte(void) {
    if (USART_GetFlagStatus(m_usart, USART_FLAG_RXNE) == SET) {
        m_rx_fifo.Put(USART_ReceiveData(m_usart));
    }
}