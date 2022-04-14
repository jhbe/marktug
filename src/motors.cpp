#include "stm32f4xx.h"
#include "motors.h"

Motors::Motors() {

    /*
     * Enable PORT C.
     */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

    /*
     * Enable Timer 8.
     */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8, ENABLE);

    /*
     * Configure Port C, pin 6 for alternative function TIMER8.
     */
    GPIO_InitTypeDef gis;
    gis.GPIO_Mode = GPIO_Mode_AF;
    gis.GPIO_OType = GPIO_OType_PP;
    gis.GPIO_Speed = GPIO_Speed_100MHz;
    gis.GPIO_PuPd = GPIO_PuPd_UP;
    gis.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_Init(GPIOC, &gis);

    GPIO_PinAFConfig(GPIOC, GPIO_PinSource6, GPIO_AF_TIM8);
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource7, GPIO_AF_TIM8);

    /*
     * Drive the timer at 1MHz with a period of 20ms. The pulse will be default
     * 1.5ms and never be outside the range 1000 - 2000 us.
     */
    TIM_TimeBaseInitTypeDef tis;
    tis.TIM_Prescaler = (SystemCoreClock / 1000000) - 1;
    tis.TIM_CounterMode = TIM_CounterMode_Up;
    tis.TIM_Period = 20000;
    tis.TIM_ClockDivision = TIM_CKD_DIV1;
    tis.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM8, &tis);

    m_oci.TIM_OCMode = TIM_OCMode_PWM1;
    m_oci.TIM_OutputState = TIM_OutputState_Enable;
    m_oci.TIM_OutputNState = TIM_OutputNState_Disable;
    m_oci.TIM_OCPolarity = TIM_OCPolarity_High;
    m_oci.TIM_OCNPolarity = TIM_OCNPolarity_Low;
    m_oci.TIM_OCIdleState = TIM_OCIdleState_Set;
    m_oci.TIM_OCNIdleState = TIM_OCNIdleState_Reset;
    m_oci.TIM_Pulse = 1500;
    TIM_OC1Init(TIM8, &m_oci);
    TIM_OC2Init(TIM8, &m_oci);

    TIM_OC1PreloadConfig(TIM8, TIM_OCPreload_Enable);
    TIM_OC2PreloadConfig(TIM8, TIM_OCPreload_Enable);

    TIM_ARRPreloadConfig(TIM8, ENABLE);

    TIM_Cmd(TIM8, ENABLE);
    TIM_CtrlPWMOutputs(TIM8, ENABLE);

    Set(Left, 0.0);
    Set(Right, 0.0);
}

/**
 * Sets the throttle of the left motor.
 *
 * @param throttle The throttle in the range -1.0 (full reverse) to 1.0 (full forward) with zero being stopped.
 */
void Motors::Set(Motor motor, float throttle) {
    /*
     * Convert to PWM pulse length in us and ensure we stay within 1000us to 2000us.
     */
    throttle = throttle * 500.0 + 1500.0;
    m_oci.TIM_Pulse = (uint32_t)(throttle > 2000.0 ? 2000.0 : (throttle < 1000.0 ? 1000.0 : throttle));

    if (motor == Left) {
        TIM_OC2Init(TIM8, &m_oci);
    } else {
        TIM_OC1Init(TIM8, &m_oci);
    }
}
