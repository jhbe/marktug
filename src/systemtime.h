#ifndef MARKTUG_SYSTEMTIME_H
#define MARKTUG_SYSTEMTIME_H

#include <stm32f4xx.h>
#include "itime.h"

/**
 * Concrete implementation of the Tiem interface using the SysTick clock.
 */
class SystemTime : public Time {
public:
    SystemTime() {
        m_ticks = 0;
        SysTick_Config(SystemCoreClock / 1000);
    }

    uint64_t Now() {
        return m_ticks;
    }

    void OnSysTick() {
        m_ticks++;
    }

    void DelayMs(uint64_t ms) {
        uint64_t start = m_ticks;
        while ((m_ticks - start) < ms);
    }

private:
    __IO uint64_t m_ticks;
};


#endif
