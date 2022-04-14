#ifndef MARKTUG_AUTOPILOT_H
#define MARKTUG_AUTOPILOT_H

#include <stdint.h>
#include "latlong.h"
#include "pi.h"

class AutoPilot {
public:
    AutoPilot() {
        m_hold_position_valid = false;
        m_distance_to_hold_position_valid = false;
        m_course_to_hold_position_valid = false;
        m_left_motor_throttle = 0.0;
        m_right_motor_throttle = 0.0;
        m_time_of_last_update = 0;
        m_forward_throttle = 0.0;
        m_max_forward_throttle_diff = 0.2; // It will take 5 seconds to go from no throttle to a throttle of 1.0.
        m_max_turning_throttle_diff = 2.0; // It will take 0.5 seconds to go from no turning to full turning throttle.

        m_zero_throttle_distance = 5.0;
        m_max_throttle_distance = 10.0;

        m_max_forward_throttle = 0.0;
        m_max_turning_throttle = 0.0;
    }

    void SetHoldPosition(LatLong position) {
        m_hold_position = position;
        m_hold_position_valid = true;
    }

    void SetMaxForwardThrottle(float max_forward_throttle) {
        m_max_forward_throttle = ClampToZeroToOne(max_forward_throttle);
    }

    void SetMaxTurningThrottle(float max_turning_throttle) {
        m_max_turning_throttle = ClampToZeroToOne(max_turning_throttle);
    }

    bool HoldPositionIsValid() {
        return m_hold_position_valid;
    }

    bool Update(LatLong current_position, bool current_position_valid, float current_heading, bool current_heading_valid, uint64_t current_time);

    float GetLeftMotorThrottle() {
        return m_left_motor_throttle;
    }

    float GetRightMotorThrottle() {
        return m_right_motor_throttle;
    }

    float GetDistanceToHoldPosition() {
        return m_distance_to_hold_position;
    }

    bool GetDistanceToHoldPositionValid() {
        return m_distance_to_hold_position_valid;
    }

    float GetCourseToHoldPosition() {
        return m_course_to_hold_position;
    }

    bool GetCourseToHoldPositionValid() {
        return m_course_to_hold_position_valid;
    }

private:
    float NormalizeToPlusMinusPi(float a) {
        while (a < -PI) {
            a += 2.0 * PI;
        }
        while (a >= PI) {
            a -= 2.0 * PI;
        }
        return a;
    }

    float ClampToZeroToOne(float v) {
        return v > 1.0 ? 1.0 : (v < 0.0 ? 0.0 : v);
    }

    float ClampToPlusMinusOne(float v) {
        return v > 1.0 ? 1.0 : (v < -1.0 ? -1.0 : v);
    }

private:
    uint64_t m_time_of_last_update;

    bool m_hold_position_valid;
    LatLong m_hold_position;

    /*
     * The calculated throttle for the left motor. Valid if the last call to Update returned true. In the range -1.0
     * (full reverse) to 1.0 (full forward).
     */
    float m_left_motor_throttle;

    /*
     * The calculated throttle for the right motor. Valid if the last call to Update returned true. In the range -1.0
     * (full reverse) to 1.0 (full forward).
     */
    float m_right_motor_throttle;

    /*
     * The amount of throttle used for forward and backward movement, i.e. excluding turning. In the range -1.0 to 1.0.
     */
    float m_forward_throttle;

    /*
     * The amount of throttle used for turning movement. In the range -1.0 to 1.0.
     */
    float m_turning_throttle;

    /*
     * A unit-less factor determining the maximum amount of forward throttle allowed. Zero means no forward throttle,
     * 1.0 means as much throttle as the motors can produce. In the range 0.0 to 1.0.
     */
    float m_max_forward_throttle;

    /*
     * A unit-less factor determining the maximum amount of throttle allowed for turning. The turning throttle is the
     * difference between the left and right throttles. Zero means no difference throttle, 1.0 means as much difference
     * as required. In the range 0.0 to 1.0;
     */
    float m_max_turning_throttle;

    /*
     * The maximum amount the forward throttle may change in one second. A non-negative number.
     */
    float m_max_forward_throttle_diff;

    /*
     * The maximum amount the turning throttle may change in one second. A non-negative number.
     */
    float m_max_turning_throttle_diff;

    /*
     * When the distance to the hold position is less than this value, then we shall use no forward throttle. A
     * non-negative value in meters.
     */
    float m_zero_throttle_distance;

    /*
     * When the distance to the hold position is less than this value but more than m_zero_throttle_distance, then we
     * shall use reduced forward throttle. A non-negative value in meters.
     */
    float m_max_throttle_distance;

    /*
     * The distance to the hold position when m_distance_to_hold_position_valid is true. A non-negative number in
     * meters.
     */
    float m_distance_to_hold_position;
    bool m_distance_to_hold_position_valid;

    /*
     * The course to the hold position when m_course_to_hold_position_valid is true. In radians in the range 0.0 <= value < 2PI.
     */
    float m_course_to_hold_position;
    bool m_course_to_hold_position_valid;
};


#endif
