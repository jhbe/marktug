#include "stm32f4xx.h"
#include "debug.h"

void Debug_Init() {
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

    GPIO_InitTypeDef gi;
    gi.GPIO_Mode = GPIO_Mode_OUT;
    gi.GPIO_OType = GPIO_OType_PP;
    gi.GPIO_PuPd = GPIO_PuPd_NOPULL;
    gi.GPIO_Speed = GPIO_Speed_50MHz;
    gi.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_15;
    GPIO_Init(GPIOA, &gi);

    GPIO_ResetBits(GPIOA, GPIO_Pin_8 | GPIO_Pin_15);
}

void S5(bool on) {
    if (on) {
        GPIO_SetBits(GPIOA, GPIO_Pin_15);
    } else {
        GPIO_ResetBits(GPIOA, GPIO_Pin_15);
    }
}

void S5Toggle() {
    GPIO_ToggleBits(GPIOA, GPIO_Pin_15);
}

void S6(bool on) {
    if (on) {
        GPIO_SetBits(GPIOA, GPIO_Pin_8);
    } else {
        GPIO_ResetBits(GPIOA, GPIO_Pin_8);
    }
}

void S6Toggle() {
    GPIO_ToggleBits(GPIOA, GPIO_Pin_8);
}
