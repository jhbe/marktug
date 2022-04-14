#ifndef MARKTUG_COMPASS_H
#define MARKTUG_COMPASS_H

#include "qmc5883.h"

class Compass {
public:
    Compass(Qmc5883 *qmc5883) {
        m_qmc5883 = qmc5883;
        m_heading_valid = false;
    }

    void Update();

    bool Heading(float &heading){
        heading = m_heading;
        return m_heading_valid;
    }

private:
    Qmc5883 *m_qmc5883;

    bool m_heading_valid;
    float m_heading;

    int16_t m_x_min, m_y_min, m_z_min;
    int16_t m_x_max, m_y_max, m_z_max;
};

#endif
