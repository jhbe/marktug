#include "qmc5883.h"
#include "led.h"
#include "itime.h"
#include "debug.h"

#define MAG_XOUT_L    0
#define MAG_XOUT_H    1
#define MAG_YOUT_L    2
#define MAG_YOUT_H    3
#define MAG_ZOUT_L    4
#define MAG_ZOUT_H    5
#define MAG_STATUS    6
#define MAG_TEMP_L    7
#define MAG_TEMP_H    8
#define MAG_CONTROL1  9
#define MAG_CONTROL2 10
#define MAG_SETRESETPERIOD 11

#define MAG_CONTROL1_STBY 0x00
#define MAG_CONTROL1_CONT 0x01

#define MAG_CONTROL1_10HZ  0x00
#define MAG_CONTROL1_50HZ  0x04
#define MAG_CONTROL1_100HZ 0x08
#define MAG_CONTROL1_200HZ 0x0c

#define MAG_CONTROL1_2G 0x00
#define MAG_CONTROL1_8G 0x10

#define MAG_CONTROL1_512 0x00
#define MAG_CONTROL1_256 0x40
#define MAG_CONTROL1_128 0x80
#define MAG_CONTROL1_64  0xc0

#define MAG_CONTROL2_SOFTRESET 0x80

Qmc5883::Qmc5883(Time *time) {
    m_i2c = new I2c(time);

    m_addr = 0x0d;

    m_i2c->Write(m_addr, MAG_CONTROL2, MAG_CONTROL2_SOFTRESET);
    time->DelayMs(1);
    m_i2c->Write(m_addr, MAG_SETRESETPERIOD, 0x01);
    time->DelayMs(1);
    m_i2c->Write(m_addr, MAG_CONTROL1, MAG_CONTROL1_CONT | MAG_CONTROL1_50HZ | MAG_CONTROL1_2G | MAG_CONTROL1_512);
}

bool Qmc5883::Read(int16_t &x, int16_t &y, int16_t &z) {
    uint8_t data[6];
    uint8_t xl, xh, yl, yh, zl, zh;
    S5Toggle();
/*    if (m_i2c->Read(m_addr, MAG_XOUT_L, xl) && m_i2c->Read(m_addr, MAG_XOUT_H, xh)
        && m_i2c->Read(m_addr, MAG_YOUT_L, yl) && m_i2c->Read(m_addr, MAG_YOUT_H, yh)
        && m_i2c->Read(m_addr, MAG_ZOUT_L, zl) && m_i2c->Read(m_addr, MAG_ZOUT_H, zh)) {
        x = xl | (xh << 8);
        y = yl | (yh << 8);
        z = zl | (zh << 8);
*/
    if (m_i2c->ReadBytes(m_addr, MAG_XOUT_L, data, 6)) {
        x = data[0] | (data[1] << 8);
        y = data[2] | (data[3] << 8);
        z = data[4] | (data[5] << 8);

        return true;
    }
    return false;
}
