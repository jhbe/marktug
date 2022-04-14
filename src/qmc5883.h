#ifndef MARKTUG_QMC5883_H
#define MARKTUG_QMC5883_H

#include <stdint.h>
#include "i2c.h"

class Qmc5883 {
public:
    Qmc5883(Time *time);

    bool Read(int16_t &x, int16_t &y, int16_t &z);

private:
    I2c *m_i2c;
    uint8_t m_addr;
};

#endif
