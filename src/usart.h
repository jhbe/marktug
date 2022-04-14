#ifndef MARKTUG__USART_H
#define MARKTUG__USART_H

#include <stdint.h>
#include "stm32f4xx.h"
#include "fifo.h"

class Usart {
public:
    Usart(USART_TypeDef* usart, uint32_t baudRate, bool fullDuplex, GPIO_TypeDef *gpio, uint16_t txPin, uint16_t rxPin, uint8_t txPinSource, uint8_t rxPinSource);

    bool Read(uint8_t &byte);
    void Write(uint8_t *buffer, uint16_t length);

    void OnInterrupt(void);

private:
    void TryToSendOneByte(void);
    void TryToReadOneByte(void);

private:
    USART_TypeDef* m_usart;
    Fifo<64> m_tx_fifo;
    Fifo<64> m_rx_fifo;
};

#endif
