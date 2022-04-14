#ifndef MARKTUG_ITIME_H
#define MARKTUG_ITIME_H

#include <stdint.h>

class Time {
public:
    virtual ~Time() {}

    /**
     * @return The current time in milliseconds since an undefined fixed epoch.
     */
    virtual uint64_t Now() = 0;

    /**
     * @param ms The number of milliseconds to block.
     */
    virtual void DelayMs(uint64_t ms) = 0;

    /**
     * @param now A time in milliseconds since a fixed undefined epoch.
     * @param then A time in milliseconds since a fixed undefined epoch.
     * @return The differance between "now" and "then" expresseed in milliseconds.
     */
    static uint64_t Age(uint64_t now, uint64_t then) {
        return now - then;
    }

    static float ToSeconds(uint64_t t) {
        return (float)t / 1000.0;
    }
};

#endif
