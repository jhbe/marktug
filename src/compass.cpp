#include <math.h>
#include <stdio.h>
#include <string.h>
#include "compass.h"
#include "stdbulkusb.h"

#define PI 3.14159265358979323846f

char buffer[1000];

void Compass::Update() {
    buffer[0] = 0;

    /*
     * Read the magnetometer.
     */
    int16_t mag_x, mag_y, mag_z;
    if (!m_qmc5883->Read(mag_x, mag_y, mag_z)) {
        sprintf(buffer, "Failed to read the qmc5883");
        UsbWrite(buffer, strlen(buffer));

        m_heading_valid = false;
        return;
    }

    m_x_min = mag_x < m_x_min ? mag_x : m_x_min;
    m_x_max = mag_x > m_x_max ? mag_x : m_x_max;

    m_y_min = mag_y < m_y_min ? mag_y : m_y_min;
    m_y_max = mag_y > m_y_max ? mag_y : m_y_max;

//    m_z_min = mag_z < m_z_min ? mag_z : m_z_min;
//    m_z_max = mag_z > m_z_max ? mag_z : m_z_max;

    int16_t x_avg = (m_x_max + m_x_min) / 2;
    int16_t y_avg = (m_y_max + m_y_min) / 2;
    int16_t x = mag_x - x_avg;
    int16_t y = mag_y - y_avg;

    m_heading = atan2(-x, -y);
    if (m_heading < 0.0) {
        m_heading += 2.0 * PI;
    }

    snprintf(buffer, 1000, "MAG: % 5i % 5i   AVG:% 5i % 5i  XY: % 5i % 5i",
             mag_x, mag_y,
             x_avg, y_avg,
             x, y);
    UsbWrite(buffer, strlen(buffer));

    m_heading_valid = true;
}