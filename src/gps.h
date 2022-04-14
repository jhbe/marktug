#ifndef MARKTUG_GPS_H
#define MARKTUG_GPS_H

#include <stdint.h>
#include "latlong.h"
#include "pi.h"

class Gps {
public:
    Gps() {
        m_positionIsValid = false;
        m_has_lock = false;
        m_bufferLength = 0;
    }

    void PutRxByte(uint8_t byte);

    bool GetPosition(LatLong &position) {
        position = m_position;
        return m_positionIsValid;
    }

    bool HasLock() {
        return m_has_lock;
    }

private:
    bool IsDigit(char c) {
        return '0' <= c && c <= '9';
    }

    double ToRadian(double deg) {
        return deg * (double)PI / (double)180.0;
    }

    double ToDeg(double rad) {
        return rad * (double)180.0 / (double)PI;
    }

    bool RadianOf(int &index, bool threeDigitDegrees, double &value);
    void ProcessGLL();
    void ProcessGSA();

private:
    bool m_positionIsValid;
    LatLong m_position;

    bool m_has_lock;

#define GPS_BUFFER_LENGTH 1000
    char m_buffer[GPS_BUFFER_LENGTH];
    int m_bufferLength;
};

#endif
