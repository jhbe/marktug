#include <gtest/gtest.h>
#include "gps.h"

class LatLongTestFixture : public ::testing::Test {
public:
    void SetUp() {
        m_from.Set(-35.0 * PI / 180.0, 135.0 * PI / 180);
        m_to.Set(-36.0 * PI / 180.0, 136.0 * PI / 180);
    }

protected:
    LatLong m_from;
    LatLong m_to;
};

TEST_F(LatLongTestFixture, DistTo) {
    ASSERT_NEAR(143642.107, m_from.DistTo(m_to), 0.01);
}

TEST_F(LatLongTestFixture, CourseTo) {
    ASSERT_NEAR(140.68 * PI / 180.0, m_from.CourseTo(m_to), 0.01 * PI / 180.0);
}

TEST_F(LatLongTestFixture, MoveTo) {
    m_from.Move(140.68 * PI / 180.0, 143642.107);
    ASSERT_NEAR(m_to.GetLongitude(), m_from.GetLongitude(), 0.000001);
    ASSERT_NEAR(m_to.GetLatitude(), m_from.GetLatitude(), 0.000001);
}
