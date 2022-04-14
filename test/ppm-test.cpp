#include <gtest/gtest.h>
#include "ppm.h"
#include "time-mock.h"

class PpmTestFixture : public ::testing::Test {
public:
    void SetUp() {
        m_ppm = new Ppm(&m_time_mock);
        m_current_time = 100;

        ON_CALL(m_time_mock, Now()).WillByDefault(::testing::InvokeWithoutArgs([&]()
                                                                               {
                                                                                   m_current_time += 1;
                                                                                   return m_current_time;
                                                                               }));
    }
    void TearDown() {
        delete m_ppm;
    }

protected:
    ::testing::NiceMock<TimeMock> m_time_mock;
    Ppm *m_ppm;

    uint64_t m_current_time;
};

TEST_F(PpmTestFixture, AtStart) {
    float value;

    /*
     * Until we get nine falling edges within 50ms, we don't have valid values.
     */
    ASSERT_FALSE(m_ppm->Get(0, value));

    m_ppm->OnFallingEdge(1500);
    ASSERT_FALSE(m_ppm->Get(0, value));

    m_ppm->OnFallingEdge(1000);
    ASSERT_FALSE(m_ppm->Get(0, value));

    m_ppm->OnFallingEdge(1500);
    ASSERT_FALSE(m_ppm->Get(0, value));

    m_ppm->OnFallingEdge(2000);
    ASSERT_FALSE(m_ppm->Get(0, value));

    m_ppm->OnFallingEdge(1500);
    ASSERT_FALSE(m_ppm->Get(0, value));

    m_ppm->OnFallingEdge(1500);
    ASSERT_FALSE(m_ppm->Get(0, value));

    m_ppm->OnFallingEdge(1500);
    ASSERT_FALSE(m_ppm->Get(0, value));

    m_ppm->OnFallingEdge(1500);
    ASSERT_FALSE(m_ppm->Get(0, value));

    m_ppm->OnFallingEdge(1500);

    /*
     * We have now measured all eight channels. We can read the values.
     */
    ASSERT_TRUE(m_ppm->Get(0, value));
    ASSERT_NEAR(-1.0, value, 1);
    ASSERT_TRUE(m_ppm->Get(1, value));
    ASSERT_NEAR(0.0, value, 1);
    ASSERT_TRUE(m_ppm->Get(2, value));
    ASSERT_NEAR(1.0, value, 1);
}

TEST_F(PpmTestFixture, SignalLoss) {
    float value;

    /*
     * Get enough falling edges to make the channels valid.
     */
    m_ppm->OnFallingEdge(1500);
    m_ppm->OnFallingEdge(1500);
    m_ppm->OnFallingEdge(1500);
    m_ppm->OnFallingEdge(1500);
    m_ppm->OnFallingEdge(1500);
    m_ppm->OnFallingEdge(1500);
    m_ppm->OnFallingEdge(1500);
    m_ppm->OnFallingEdge(1500);
    m_ppm->OnFallingEdge(1500);

    /*
     * Two more, then signal loss for 1s. After that the channels shall be invalid.
     */
    m_ppm->OnFallingEdge(1500);
    m_ppm->OnFallingEdge(1500);
    ASSERT_TRUE(m_ppm->Get(2, value));
    m_current_time += 1000;
    ASSERT_FALSE(m_ppm->Get(0, value));

    /*
     * Having lost the signal, we now need nine fresh falling edges.
     */
    m_ppm->OnFallingEdge(1500);
    ASSERT_FALSE(m_ppm->Get(0, value));
    m_ppm->OnFallingEdge(1000);
    ASSERT_FALSE(m_ppm->Get(0, value));
    m_ppm->OnFallingEdge(1500);
    ASSERT_FALSE(m_ppm->Get(0, value));
    m_ppm->OnFallingEdge(2000);
    ASSERT_FALSE(m_ppm->Get(0, value));
    m_ppm->OnFallingEdge(1500);
    ASSERT_FALSE(m_ppm->Get(0, value));
    m_ppm->OnFallingEdge(1500);
    ASSERT_FALSE(m_ppm->Get(0, value));
    m_ppm->OnFallingEdge(1500);
    ASSERT_FALSE(m_ppm->Get(0, value));
    m_ppm->OnFallingEdge(1500);
    ASSERT_FALSE(m_ppm->Get(0, value));
    m_ppm->OnFallingEdge(1500);
    ASSERT_TRUE(m_ppm->Get(0, value));

}