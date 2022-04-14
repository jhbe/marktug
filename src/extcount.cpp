#include "stm32f4xx.h"
#include "extcount.h"
#include <led.h>
#include <stdio.h>
#include <string.h>
#include <stdbulkusb.h>

ExtCount::ExtCount(TIM_TypeDef *timer, uint8_t channel, uint16_t interruptSource, GPIO_TypeDef *gpio, uint16_t pin,
                   uint8_t pinSource) {
    m_timer = timer;
    m_channel = channel;
    m_gpio = gpio;
    m_pin = pin;
    m_pinSource = pinSource;
    m_interruptSource = interruptSource;

    if (m_gpio == GPIOA) {
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    } else if (m_gpio == GPIOB) {
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    } else if (m_gpio == GPIOC) {
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
    } else {
        assert_param(false);
    }

    if (m_timer == TIM2) {
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    } else if (m_timer == TIM3) {
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    } else if (m_timer == TIM4) {
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
    } else if (m_timer == TIM5) {
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);
    } else {
        assert_param(false);
    }

    GPIO_InitTypeDef gis;
    gis.GPIO_Mode = GPIO_Mode_AF;
    gis.GPIO_OType = GPIO_OType_PP;
    gis.GPIO_Speed = GPIO_Speed_100MHz;
    gis.GPIO_PuPd = GPIO_PuPd_NOPULL;
    gis.GPIO_Pin = m_pin;
    GPIO_Init(m_gpio, &gis);

    if (m_timer == TIM2) {
        GPIO_PinAFConfig(m_gpio, m_pinSource, GPIO_AF_TIM2);
    } else if (m_timer == TIM3) {
        GPIO_PinAFConfig(m_gpio, m_pinSource, GPIO_AF_TIM3);
    } else if (m_timer == TIM4) {
        GPIO_PinAFConfig(m_gpio, m_pinSource, GPIO_AF_TIM4);
    } else if (m_timer == TIM5) {
        GPIO_PinAFConfig(m_gpio, m_pinSource, GPIO_AF_TIM5);
    } else {
        assert_param(false);
    }

    TIM_TimeBaseInitTypeDef tis;
    tis.TIM_Prescaler = (SystemCoreClock / 2000000) - 1;
    tis.TIM_CounterMode = TIM_CounterMode_Up;
    tis.TIM_Period = 0xFFFFFFFF;
    tis.TIM_ClockDivision = TIM_CKD_DIV1;
    tis.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(m_timer, &tis);

    TIM_ICInitTypeDef ici;
    if (m_channel == 1) {
        ici.TIM_Channel = TIM_Channel_1;
    } else if (m_channel == 2) {
        ici.TIM_Channel = TIM_Channel_2;
    } else if (m_channel == 3) {
        ici.TIM_Channel = TIM_Channel_3;
    } else if (m_channel == 4) {
        ici.TIM_Channel = TIM_Channel_4;
    }
    ici.TIM_ICPolarity = TIM_ICPolarity_Falling;
    ici.TIM_ICSelection = TIM_ICSelection_DirectTI;
    ici.TIM_ICPrescaler = TIM_ICPSC_DIV1;
    ici.TIM_ICFilter = 0;
    TIM_ICInit(m_timer, &ici);

    NVIC_InitTypeDef ii;
    if (m_timer == TIM2) {
        ii.NVIC_IRQChannel = TIM2_IRQn;
    } else if (m_timer == TIM3) {
        ii.NVIC_IRQChannel = TIM3_IRQn;
    } else if (m_timer == TIM4) {
        ii.NVIC_IRQChannel = TIM4_IRQn;
    } else if (m_timer == TIM5) {
        ii.NVIC_IRQChannel = TIM5_IRQn;
    }
    ii.NVIC_IRQChannelCmd = ENABLE;
    ii.NVIC_IRQChannelPreemptionPriority = 0;
    ii.NVIC_IRQChannelSubPriority = 1;
    NVIC_Init(&ii);

    TIM_Cmd(m_timer, ENABLE);
    TIM_ITConfig(m_timer, m_interruptSource, ENABLE);
}

void ExtCount::OnInterrupt(void) {
    if (TIM_GetITStatus(m_timer, m_interruptSource) == SET) {
        if (m_channel == 1) {
            m_count = TIM_GetCapture1(m_timer);
        } else if (m_channel == 2) {
            m_count = TIM_GetCapture2(m_timer);
        } else if (m_channel == 3) {
            m_count = TIM_GetCapture3(m_timer);
        } else if (m_channel == 4) {
            m_count = TIM_GetCapture4(m_timer);
        }
        TIM_SetCounter(m_timer, 0);

        TIM_ClearITPendingBit(m_timer, m_interruptSource);
    }
}

