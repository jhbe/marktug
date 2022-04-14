#ifndef MARKTUG_MOTORS_H
#define MARKTUG_MOTORS_H

#include <stdint.h>
#include "stm32f4xx.h"

#define MOTOR_ZERO_THROTTLE 1500

class Motors {
public:
    Motors();

    enum Motor {
        Left,
        Right
    };
    void Set(Motor motor, float throttle);

private:
    TIM_OCInitTypeDef m_oci;
};

#endif
