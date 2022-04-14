#include "stm32f4xx.h"
#include "led.h"
#include "stdbulkusb.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "usart.h"
#include "fifo.h"
#include "qmc5883.h"
#include "extcount.h"
#include "ppm.h"
#include "ibus.h"
#include "gps.h"
#include "motors.h"
#include "compass.h"
#include "autopilot.h"
#include "debug.h"
#include "systemtime.h"

Usart *gpsUsart = nullptr;
Usart *ibusUsart = nullptr;
Fifo<1000> gpsFifo;
Fifo<1000> ibusFifo;

SystemTime *time = nullptr;
ExtCount *extCount = nullptr;
Ppm *ppm = nullptr;
Ibus *ibus = nullptr;
Gps *gps = nullptr;

void onRxCallback(char *buffer, int length) {

}

extern "C"
void USART1_IRQHandler(void) {
    gpsUsart->OnInterrupt();

    uint8_t byte;
    while (gpsUsart->Read(byte)) {
        gpsFifo.Put(byte);
    }
}

extern "C"
void UART4_IRQHandler(void) {
    ibusUsart->OnInterrupt();

    uint8_t byte;
    while (ibusUsart->Read(byte)) {
        ibusFifo.Put(byte);
    }
}

extern "C"
void TIM5_IRQHandler(void) {
    extCount->OnInterrupt();
    ppm->OnFallingEdge(extCount->Get());
}

/**
 * SysTick interrupt handler. Will be invoked at 1kHz.
 */
extern "C"
void SysTick_Handler(void) {
    time->OnSysTick();
}

enum Mode {
    /**
     * The boat is under direct control from the radio. No auto pilot involved.
     */
    Manual,

    /**
     * The auto pilot will control the motors and keep the boat at the nominated latitude/longitude.
     */
    Hold,
};

int main() {
    time = new SystemTime();
    Debug_Init();
    LedInit();

    time->DelayMs(1000);

    UsbInit(onRxCallback);

    gpsUsart = new Usart(USART1, 9600, true, GPIOA, GPIO_Pin_9, GPIO_Pin_10, GPIO_PinSource9, GPIO_PinSource10);
    gps = new Gps();
    ibusUsart = new Usart(UART4, 115200, false, GPIOA, GPIO_Pin_0, 0, GPIO_PinSource0, 0);
    ibus = new Ibus(ibusUsart);
    extCount = new ExtCount(TIM5, 4, TIM_IT_CC4, GPIOA, GPIO_Pin_3, GPIO_PinSource3);
    ppm = new Ppm(time);

    Motors motors;
    Qmc5883 mag(time);
    Compass compass(&mag);
    AutoPilot auto_pilot;

    //char buffer[1000];
    //buffer[0] = 0;

    Mode mode = Manual;

    uint32_t last_time = 0;
    while (1) {
        /*
         * Update received GPS and IBUS telemetry bytes. This need to happen quickly and often, otherwise the IBUS
         * won't be able to maintain the required timing. Less critical for the GPS.
         */
        uint8_t b;
        while (ibusFifo.Get(b) && ibus != nullptr) {
            ibus->PutRxByte(b);
        }
        while (gpsFifo.Get(b) && gps != nullptr) {
            gps->PutRxByte(b);
        }

        uint64_t now = time->Now();
        if (SystemTime::Age(now, last_time) > 50) {

            /*
             * Read the magnetometer.
             */
            compass.Update();
            float heading;
            bool heading_valid = compass.Heading(heading);

            /*
             * Read the GPS.
             */
            LatLong position;
            bool position_valid = gps->GetPosition(position);

            /*
             * Update the auto pilot adjustable parameters, specifically the maximum allowed forward throttle and
             * turning throttle. The PPM output is in the range -1.0 to 1.0, but we need 0.0 to 1.0 for the max
             * throttles.
             */
            float channel;
            if (ppm->Get(2, channel)) {
                auto_pilot.SetMaxForwardThrottle(channel / 2.0 + 0.5);
            }
            if (ppm->Get(3, channel)) {
                auto_pilot.SetMaxTurningThrottle(channel / 2.0 + 0.5);
            }

            /*
             * Read the receiver mode channel and update the mode.
             */
            float mode_channel;
            bool mode_channel_valid = ppm->Get(4, mode_channel);
            if (mode_channel_valid) {
                if (mode_channel < -0.5) {
                    if (mode == Manual && position_valid && gps->HasLock()) {
                        /*
                         * We currently have a valid position, so we can transition from manual to Hold. The current position
                         * is the one we're meant to stay at.
                         */
                        auto_pilot.SetHoldPosition(position);
                        mode = Hold;
                    }
                } else if (mode_channel < 0.5) {
                    mode = Manual;
                } else /* mode_channel > 0.5 */ {
                    if (auto_pilot.HoldPositionIsValid()) {
                        /*
                         * The auto pilot has a valid hold position, so we can transition to Hold mode.
                         */
                        mode = Hold;
                    }
                }
            }

            if (mode == Manual || !mode_channel_valid) {
                motors.Set(motors.Left, ppm->Get(0, channel) ? channel : 0.0);
                motors.Set(motors.Right, ppm->Get(1, channel) ? channel : 0.0);
            } else {
                auto_pilot.Update(position, position_valid, heading, heading_valid, time->Now());
                motors.Set(motors.Left, auto_pilot.GetLeftMotorThrottle());
                motors.Set(motors.Right, auto_pilot.GetRightMotorThrottle());
            }

            /*
             * Provide telemetry data.
             */
            ibus->SetPosition(position.GetLatitude(), position.GetLongitude(), position_valid);
            ibus->SetHeading(heading, heading_valid);
            ibus->SetCog(auto_pilot.GetCourseToHoldPosition(), auto_pilot.GetCourseToHoldPositionValid());
            ibus->SetSpeed(10, true);
            ibus->SetDistance(auto_pilot.GetDistanceToHoldPosition(), auto_pilot.GetDistanceToHoldPositionValid());

            last_time = now;
            LedBlueToggle();

/*            if (position_valid) {
                sprintf(buffer, "% 9.6f   % 9.6f ", position.getLatitude() * (double) 180.0 / (double) PI,
                        position.getLongitude() * (double) 180.0 / (double) PI);
            } else {
                sprintf(buffer, "No position");
            }
            */


            //sprintf(buffer, "%s  PPM:%s  %03.0f", mode == Manual ? "Manual" : "Hold", mode_channel_valid ? "VALID" : "invalid", heading_valid ? heading * 180.0f / PI : -1.0f);
            //UsbWrite(buffer, strlen(buffer));

        }
    }

}

void assert_failed(uint8_t *file, uint32_t line) {
    LedRedOn();
    LedBlueOff();
    while (1) {
        time->DelayMs(100);
        LedRedToggle();
        LedBlueToggle();
    }
}
