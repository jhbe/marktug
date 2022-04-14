#include <gtest/gtest.h>
#include "autopilot.h"
#include "gps.h"

float ToSeconds(uint64_t t) {
    return (float) t / 1000.0;
}

class AutoPilotTestFixture : public ::testing::Test {
public:
    void SetUp() {
        m_hold_position.Set(-35.0 * PI / 180, 135.0 * PI / 180);

        /*
         * One degree is 1852*60=111120m at the equator. At 35 deg latitude a degree longitude is 91024m.
         *
         * So a position 11 meters to the south and 9m to the east is:
         */
        m_current_position.Set(-35.0001 * PI / 180, 135.0001 * PI / 180);
    }

    void TearDown() {}

protected:
    AutoPilot m_ap;
    LatLong m_hold_position;
    LatLong m_current_position;
};

TEST_F(AutoPilotTestFixture, Defaults) {
    ASSERT_FALSE(m_ap.HoldPositionIsValid());
    ASSERT_FALSE(m_ap.Update(LatLong(), false, 0.0, false, 1));
    ASSERT_FALSE(m_ap.Update(LatLong(), true, 0.0, false, 1));
    ASSERT_FALSE(m_ap.Update(LatLong(), true, 0.0, false, 1));
    ASSERT_FALSE(m_ap.Update(LatLong(), true, 0.0, true, 1));
}

TEST_F(AutoPilotTestFixture, UpdateWithHoldPosition) {
    m_ap.SetHoldPosition(m_hold_position);
    ASSERT_TRUE(m_ap.HoldPositionIsValid());
    ASSERT_FALSE(m_ap.Update(LatLong(), false, 0.0, false, 1));
    ASSERT_FALSE(m_ap.Update(LatLong(), false, 0.0, true, 1));
    ASSERT_FALSE(m_ap.Update(LatLong(), true, 0.0, false, 1));
    ASSERT_TRUE(m_ap.Update(LatLong(), true, 0.0, true, 1));
}

TEST_F(AutoPilotTestFixture, TurnAround) {
    m_ap.SetHoldPosition(m_hold_position);
    m_ap.SetMaxForwardThrottle(0.2);
    m_ap.SetMaxTurningThrottle(2.0);
    m_ap.Update(m_current_position, true, 180 * PI / 180, true, 100);

    /*
     * Facing away from the hold position we expect a zero throttle forward and some throttle to turn around; i.e. the
     * left and right throttle shall be opposite each other. We expect it to turn clockwise, so left throttle shall be
     * positive.
     */
    float left = m_ap.GetLeftMotorThrottle();
    float right = m_ap.GetRightMotorThrottle();
    ASSERT_NEAR(left, 0.01, 0.01);
    ASSERT_NEAR(right, -0.01, 0.01);
}

TEST_F(AutoPilotTestFixture, StartMovingForward) {
    m_ap.SetHoldPosition(m_hold_position);
    m_ap.SetMaxForwardThrottle(0.2);
    m_ap.SetMaxTurningThrottle(2.0);
    m_ap.Update(m_current_position, true, 0 * PI / 180, true, 100);
    m_ap.Update(m_current_position, true, 0 * PI / 180, true, 200);

    /*
     * With the position to hold about 45 deg to the left, we expect some turning throttle and a bit of forward throttle.
     */
    float left = m_ap.GetLeftMotorThrottle();
    float right = m_ap.GetRightMotorThrottle();
    ASSERT_NEAR(left, -0.18, 0.01);
    ASSERT_NEAR(right, 0.22, 0.01);
}
/*
TEST_F(AutoPilotTestFixture, ProceedAndHold) {
    m_ap.SetHoldPosition(m_hold_position);
    m_ap.SetMaxForwardThrottle(0.2);
    m_ap.SetMaxTurningThrottle(2.0);

    uint64_t time = 0;
    float course = PI;
    float speed = 0.0;
    for (int i = 0; i < 10000; i++) {
        m_ap.Update(m_current_position, true, course, true, time);

        float left = m_ap.GetLeftMotorThrottle();
        float right = m_ap.GetRightMotorThrottle();

        course += (left - right) * 1.0 * 0.1;
        while (course < 0.0) {
            course += 2.0 * PI;
        }
        while (course > 2.0 * PI) {
            course -= 2.0 * PI;
        }
        speed += (left + right) / 2.0 * 1.0;
        speed *= 0.9;

        m_current_position.Move(course, speed * 0.1);
        m_current_position.Move(PI, 0.01);

        printf("%03.0f  % 6.1f\n", course * 180.0 / PI, speed);

        time += 100;
    }
}
*/