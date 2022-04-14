#include <math.h>
#include "latlong.h"

/*
 * 180.0 / PI * 60.0 * 1852.0
 */
#define RADIANS_TO_METERS 6366707.019493707
#define METERS_TO_RADIANS 0.0000001570670673141045335604473333773

double LatLong::DistTo(LatLong to) {
    double dx = (to.m_long - m_long) * cos(m_lat);
    double dy = to.m_lat - m_lat;

    return sqrt(dx*dx+dy*dy) * (double)RADIANS_TO_METERS;
}

double LatLong::CourseTo(LatLong to) {
    double dx = (to.m_long - m_long) * cos(m_lat);
    double dy = to.m_lat - m_lat;

    return atan2(dx, dy);
}

void LatLong::Move(float course, float distance) {
    m_long += (double)distance * sin((double)course) * (double)METERS_TO_RADIANS / cos((double)m_lat);
    m_lat += (double)distance * cos((double)course) * (double)METERS_TO_RADIANS;
}
