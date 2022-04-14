#include <gtest/gtest.h>
#include "gps.h"

class GpsTestFixture : public ::testing::Test {
public:
    void SetUp() {}
    void TearDown() {}

    void Feed(const char *buffer) {
        for (int i = 0; i < strlen(buffer); i++) {
            m_gps.PutRxByte(buffer[i]);
        }
    }

protected:
    Gps m_gps;
    uint8_t m_byte;
};

TEST_F(GpsTestFixture, Defaults) {
    LatLong position;
    ASSERT_FALSE(m_gps.GetPosition(position));
    ASSERT_FALSE(m_gps.HasLock());
}

TEST_F(GpsTestFixture, EmptyGLL) {
    Feed("$GNGLL,,,,,,V,N*7A");
    LatLong position;
    ASSERT_FALSE(m_gps.GetPosition(position));
}

TEST_F(GpsTestFixture, ValidGLLTwoDecimals) {
    Feed("$GNGLL,3454.11,S,13829.77,E,070238.00,A,A*67");
    LatLong position;
    ASSERT_TRUE(m_gps.GetPosition(position));
    ASSERT_NEAR(-0.609151907, position.GetLatitude(), 0.0001);
    ASSERT_NEAR(2.41721411, position.GetLongitude(), 0.0001);
}

TEST_F(GpsTestFixture, ValidGLLFiveDecimals) {
    Feed("$GNGLL,3454.11899,S,13829.77914,E,070238.00,A,A*67");
    LatLong position;
    ASSERT_TRUE(m_gps.GetPosition(position));
}

TEST_F(GpsTestFixture, EmptyGSA) {
    Feed("$GNGSA,A,1,,,,,,,,,,,,,99.99,99.99,99.99*");
    ASSERT_FALSE(m_gps.HasLock());
}

TEST_F(GpsTestFixture, FullGSA) {
    Feed("$GNGSA,A,3,02,12,29,24,20,25,,,,,,,2.32,1.22,1.98*");
    ASSERT_TRUE(m_gps.HasLock());
}