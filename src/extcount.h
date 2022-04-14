#ifndef MARKTUG_EXTCOUNT_H
#define MARKTUG_EXTCOUNT_H

class ExtCount {
public:
    ExtCount(TIM_TypeDef* timer, uint8_t channel, uint16_t interruptSource, GPIO_TypeDef *gpio, uint16_t pin, uint8_t pinSource);

    void OnInterrupt(void);

    uint32_t Get(void) {
        return m_count;
    }

private:
    TIM_TypeDef* m_timer;
    uint8_t m_channel;
    GPIO_TypeDef *m_gpio;
    uint16_t m_pin;
    uint8_t m_pinSource;
    uint16_t m_interruptSource;

    uint32_t m_count;
};

#endif
