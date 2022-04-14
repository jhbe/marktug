#include "ppm.h"

Ppm::Ppm(Time *time) {
    m_time = time;
    m_time_of_last_falling_edge = 0;
    m_valid = false;

    for (int i = 0; i < NUMBER_OF_CHANNELS; i++) {
        m_channels[i] = 0;
    }
    m_currentChannel = NO_CHANNEL;
}

void Ppm::OnFallingEdge(uint16_t us) {
    if (us < 0) {
        return;
    }

    m_time_of_last_falling_edge = m_time->Now();
    if (us > 3000) {
        m_currentChannel = 0;
    } else {
        if (0 <= m_currentChannel && m_currentChannel < NUMBER_OF_CHANNELS) {
            m_channels[m_currentChannel] = us;
        }
        if (m_currentChannel == NUMBER_OF_CHANNELS - 1) {
            m_valid = true;
            m_currentChannel = NO_CHANNEL;
        } else {
            m_currentChannel++;
        }
    }
}

bool Ppm::Get(uint8_t channel, float &value) {
    if (channel < 0 || NUMBER_OF_CHANNELS <= channel) {
        return false;
    }
    if (m_time->Now() - m_time_of_last_falling_edge > 50) {
        m_valid = false;
        m_currentChannel = NO_CHANNEL;
        return false;
    }
    value = ((float)m_channels[channel] - 1500.0) / 500.0;
    return m_valid;
}
