#ifndef MARKTUG_LATLONG_H
#define MARKTUG_LATLONG_H

class LatLong {
public:
    LatLong() {
        m_lat = 0.0;
        m_long = 0.0;
    }

    void Set(double lat, double lon) {
        m_lat = lat;
        m_long = lon;
    }

    double GetLatitude() {
        return m_lat;
    }

    double GetLongitude() {
        return m_long;
    }

    double DistTo(LatLong to);

    double CourseTo(LatLong toLatLong);

    void Move(float course, float distance);

private:
    double m_lat;  // Radians
    double m_long; // Radians
};

#endif
