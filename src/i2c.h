#ifndef MARKTUG_I2C_H
#define MARKTUG_I2C_H

#include <stdint.h>
#include "itime.h"

class I2c {
public:
    I2c(Time *time);

    bool Write(uint8_t addr, uint8_t reg, uint8_t data);
    bool ReadBytes(uint8_t addr, uint8_t reg, uint8_t *data, uint8_t length);

private:
    bool WaitFor(uint32_t condition);

private:
    Time *m_time;
};


#endif
