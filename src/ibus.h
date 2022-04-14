#ifndef MARKTUG_IBUS_H
#define MARKTUG_IBUS_H

#include <stdint.h>
#include "usart.h"

#define MAX_NUMBER_OF_CHANNELS 6

class Ibus {
public:
    Ibus(Usart *usart) {
        m_usart = usart;
        m_length = 0;

        m_position_valid = false;
        m_heading_valid = false;
        m_cog_valid = false;
        m_speed_valid = false;
        m_distance_valid = false;
        m_servo_valid = false;
    }

    void SetPosition(double latitude, double longitude, bool valid) {
        m_latitude = latitude;
        m_longitude = longitude;
        m_position_valid = valid;
    }

    void SetHeading(float heading, bool valid) {
        m_heading = heading;
        m_heading_valid = valid;
    }

    void SetCog(float cog, bool valid) {
        m_cog = cog;
        m_cog_valid = valid;
    }

    void SetSpeed(float speed, bool valid) {
        m_speed = speed;
        m_speed_valid = valid;
    }

    void SetDistance(float distance, bool valid) {
        m_distance = distance;
        m_distance_valid = valid;
    }

    bool Get(int8_t channel, int16_t &servo) {
        if (0 < channel || channel > MAX_NUMBER_OF_CHANNELS) {
            return false;
        }
        servo = m_servo[channel];
        return m_servo_valid;
    }

    void PutRxByte(uint8_t byte);

private:
    void Send(uint8_t *buffer, uint16_t length);
    void Send(uint8_t byte);

    void ProcessServoPacket(uint8_t *buffer, uint16_t length);
    void ProcessTelemetryPacket(uint8_t *buffer, uint16_t length);

private:
    uint8_t m_rx_buffer[64];
    uint16_t m_length;

    double m_latitude;
    double m_longitude;
    bool m_position_valid;

    float m_heading;
    bool m_heading_valid;

    float m_cog;
    bool m_cog_valid;

    float m_speed;
    bool m_speed_valid;

    float m_distance;
    bool m_distance_valid;

    int16_t m_servo[MAX_NUMBER_OF_CHANNELS];
    bool m_servo_valid;

    Usart *m_usart;
};

#endif
