#include <gtest/gtest.h>
#include "fifo.h"

class FifoTestFixture : public ::testing::Test {
public:
    void SetUp() {}
    void TearDown() {}

protected:
    Fifo<4> m_fifo;
    uint8_t m_byte;
};

TEST_F(FifoTestFixture, Defaults) {
    /*
     * Shall initialize to empty.
     */
    ASSERT_FALSE(m_fifo.Get(m_byte));
}

TEST_F(FifoTestFixture, ZeroLengthFifo) {
    Fifo<0> zeroFifo;

    /*
     * Can't do much with a zero-length FIFO.
     */
    ASSERT_FALSE(zeroFifo.Put(1));
    ASSERT_FALSE(zeroFifo.Get(m_byte));
}

TEST_F(FifoTestFixture, Full) {
    /*
     * The FIFO is of size 4, which allows up to 4 bytes.
     */
    ASSERT_TRUE(m_fifo.Put(1));
    ASSERT_TRUE(m_fifo.Put(2));
    ASSERT_TRUE(m_fifo.Put(3));
    ASSERT_TRUE(m_fifo.Put(4));
    ASSERT_FALSE(m_fifo.Put(5));
}

TEST_F(FifoTestFixture, SingleByte) {
    m_fifo.Put(1);
    ASSERT_TRUE(m_fifo.Get(m_byte));
    ASSERT_EQ(m_byte, 1);
}

TEST_F(FifoTestFixture, ManyBytes) {
    /*
     * Put two bytes in and retrieve them.
     */
    m_fifo.Put(11);
    m_fifo.Put(12);
    ASSERT_TRUE(m_fifo.Get(m_byte));
    ASSERT_EQ(m_byte, 11);
    ASSERT_TRUE(m_fifo.Get(m_byte));
    ASSERT_EQ(m_byte, 12);

    /*
     * Put three more bytes in and retrieve them. This will test the wrap around in the buffer.
     */
    m_fifo.Put(13);
    m_fifo.Put(14);
    m_fifo.Put(15);
    ASSERT_TRUE(m_fifo.Get(m_byte));
    ASSERT_EQ(m_byte, 13);
    ASSERT_TRUE(m_fifo.Get(m_byte));
    ASSERT_EQ(m_byte, 14);
    ASSERT_TRUE(m_fifo.Get(m_byte));
    ASSERT_EQ(m_byte, 15);

    /*
     * The fifo is empty.
     */
    ASSERT_FALSE(m_fifo.Get(m_byte));
}
