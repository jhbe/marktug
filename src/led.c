#include "stm32f4xx.h"
#include "led.h"

#define  RED_LED_PIN  GPIO_Pin_14
#define BLUE_LED_PIN  GPIO_Pin_9
#define  RED_LED_PORT GPIOA
#define BLUE_LED_PORT GPIOB

void LedInit(void) {
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

    GPIO_InitTypeDef gis;
    gis.GPIO_Mode = GPIO_Mode_OUT;
    gis.GPIO_OType = GPIO_OType_PP;
    gis.GPIO_Speed = GPIO_Speed_100MHz;
    gis.GPIO_PuPd = GPIO_PuPd_NOPULL;
    gis.GPIO_Pin = RED_LED_PIN;
    GPIO_Init(RED_LED_PORT, &gis);
    gis.GPIO_Pin = BLUE_LED_PIN;
    GPIO_Init(BLUE_LED_PORT, &gis);

    /*
     * Turn off the red and blue LED.
     */
    GPIO_SetBits(RED_LED_PORT, RED_LED_PIN);
    GPIO_SetBits(BLUE_LED_PORT, BLUE_LED_PIN);
}

void LedRedOn(void) {
    GPIO_ResetBits(RED_LED_PORT, RED_LED_PIN);
}

void LedBlueOn(void) {
    GPIO_ResetBits(BLUE_LED_PORT, BLUE_LED_PIN);
}

void LedRedOff(void) {
    GPIO_SetBits(RED_LED_PORT, RED_LED_PIN);
}

void LedBlueOff(void) {
    GPIO_SetBits(BLUE_LED_PORT, BLUE_LED_PIN);
}

void LedRedToggle(void) {
    GPIO_ToggleBits(RED_LED_PORT, RED_LED_PIN);
}

void LedBlueToggle(void) {
    GPIO_ToggleBits(BLUE_LED_PORT, BLUE_LED_PIN);
}
