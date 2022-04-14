#ifndef MARKTUG_PPM_H
#define MARKTUG_PPM_H

#include <stdint.h>
#include "itime.h"

class Ppm {
public:
    Ppm(Time *time);

    /**
     * Invoke whenever the PPM signal goes from high to low.
     *
     * @param us The time since the last falling edge in micro seconds. Must be non-negative.
     */
    void OnFallingEdge(uint16_t us);

    /**
     * @param channel The channel. Aileron is zero, elevator is one, etc. valid channels are 0 - 7 inclusive.
     * @param value The stick position in the range -1.0 to 1.0 if the return value is true. Zero is stick in the middle.
     * @return True if value was set, false otherwise.
     */
    bool Get(uint8_t channel, float &value);

private:
    Time *m_time;

    /*
     * If true, then m_channels are valid.
     */
    bool m_valid;

    /*
     * The time when the last falling edge was received. Used to detect when the signal is lost completely.
     */
    uint64_t m_time_of_last_falling_edge;

#define NUMBER_OF_CHANNELS 8
    /*
     * The last measured length in micro seconds of each channel. Typical values range from 1000 to 2000.
     */
    uint16_t m_channels[NUMBER_OF_CHANNELS];

#define NO_CHANNEL -1
    /*
     * The channel we are measuring. When the next falling edge arrives, then we have measured the current channel.
     * "NO_CHANNEL" means we are awaiting the falling edge that signals the start of the first ("0") channel.
     */
    int8_t m_currentChannel;
};

#endif
