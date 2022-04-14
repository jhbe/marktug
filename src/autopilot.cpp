#include <math.h>
#include "autopilot.h"
#include "itime.h"

bool AutoPilot::Update(LatLong current_position, bool current_position_valid, float current_heading,
                       bool current_heading_valid, uint64_t current_time) {
    /*
     * Do we have the information we need?
     */
    if (!m_hold_position_valid || !current_position_valid || !current_heading_valid) {
        m_left_motor_throttle = 0.0;
        m_right_motor_throttle = 0.0;
        m_distance_to_hold_position_valid = false;
        m_course_to_hold_position_valid = false;

        return false;
    }

    /*
     * Calculate the delta time since the last update. If this is the first time we do this, then the delta time is zero.
     */
    float dt = Time::ToSeconds(m_time_of_last_update == 0 ? 0 : current_time - m_time_of_last_update);
    m_time_of_last_update = current_time;

    /*
     * Calculate the course we need to hold and distance we need to cover.
     */
    m_distance_to_hold_position = current_position.DistTo(m_hold_position);
    m_distance_to_hold_position_valid = true;
    m_course_to_hold_position = current_position.CourseTo(m_hold_position);
    m_course_to_hold_position_valid = true;

    /*
     * Calculate the difference in the course we need to keep and the course we're currently keeping.
     */
    float course_diff = NormalizeToPlusMinusPi(m_course_to_hold_position - current_heading);

    /*
     * Calculate the turning_throttle; i.e. the amount of differential motor throttle that will turn the boat around.
     * If we're facing away from where we need to go, then use the allowed throttle. If we're less than 90 deg out,
     * then use less throttle the closer to the correct course we are.
     */
    float new_turning_throttle = m_max_turning_throttle;
    if (course_diff < -PI / 2.0) {
        /*
         * Turn left, not right.
         */
        new_turning_throttle *= 1.0;
    } else if (course_diff < PI / 2.0) {
        new_turning_throttle *= sin(course_diff);
    }

    /*
     * Calculate the desired forward throttle. If we are more than 90 degrees of the course we need to keep, then we want no
     * throttle and will use motors only to turn the correct way. If we are inside 90 deg, then the closer to the correct
     * course we are the more throttle we allow.
     */
    float new_forward_throttle = m_max_forward_throttle;
    if (-PI / 2.0 < course_diff && course_diff < PI / 2.0) {
        new_forward_throttle *= cos(course_diff);
    } else {
        new_forward_throttle = 0.0;
    }

    /*
     * As we get closer to the desired position, we decrease throttle. When we're near the position we shall have no
     * throttle. The idea is that it is better to stop before the hold position and maintain the heading than to
     * overshoot and have to do a U-turn.
     */
    if (m_distance_to_hold_position < m_zero_throttle_distance) {
        new_forward_throttle = 0.0;
    } else if (m_distance_to_hold_position < m_max_throttle_distance) {
        new_forward_throttle *= (m_distance_to_hold_position - m_zero_throttle_distance) /
                (m_max_throttle_distance - m_zero_throttle_distance);
    }

    /*
     * There must be no sudden moves forwards or backwards, so we apply a lowpass filter on the forward throttle.
     */
    float forward_throttle_diff = new_forward_throttle - m_forward_throttle;
    if (forward_throttle_diff < -m_max_forward_throttle_diff * dt) {
        m_forward_throttle -= m_max_forward_throttle_diff * dt;
    } else if (forward_throttle_diff > m_max_forward_throttle_diff * dt) {
        m_forward_throttle += m_max_forward_throttle_diff * dt;
    } else {
        m_forward_throttle = new_forward_throttle;
    }

    /*
     * We apply a lowpass filter to the turning throttle as well, just to avoid jerky movement and give the motors a chance to spin up/down smoothly.
     */
    float turning_throttle_diff = new_turning_throttle - m_turning_throttle;
    if (turning_throttle_diff < -m_max_turning_throttle_diff * dt) {
        m_turning_throttle -= m_max_turning_throttle_diff * dt;
    } else if (turning_throttle_diff > m_max_turning_throttle_diff * dt) {
        m_turning_throttle += m_max_turning_throttle_diff * dt;
    } else {
        m_turning_throttle = new_turning_throttle;
    }

    /*
     * Convert forward and turning throttle to motor outputs and ensure we're not exceeding min/max throttle.
     */
    m_left_motor_throttle = ClampToPlusMinusOne(m_forward_throttle + m_turning_throttle);
    m_right_motor_throttle = ClampToPlusMinusOne(m_forward_throttle - m_turning_throttle);

    return true;
}
