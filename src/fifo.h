#ifndef MARKTUG_FIFO_H
#define MARKTUG_FIFO_H

#include <stdint.h>

/**
 * The purpose of this class is to provide a first-in-first-out queue.
 *
 * @tparam FIFO_SIZE The size of the queue. Must be zero or larger.
 */
template<uint16_t FIFO_SIZE>
class Fifo {
public:
    Fifo() {
        m_start = 0;
        m_end = 0;
    }

    /**
     * Adds a byte to the FIFO.
     *
     * @param byte The byte to add.
     * @return True if the byte was successfully added, false otherwise.
     */
    bool Put(uint8_t byte) {
        if (m_end == m_start - 1 || (m_end == FIFO_SIZE && m_start == 0)) {
            return false;
        }
        m_buffer[m_end++] = byte;
        if (m_end == FIFO_SIZE + 1) {
            m_end = 0;
        }
        return true;
    }

    /**
     * Pulls a byte from the fifo.
     *
     * @param byte The retrieved byte if the function returned true. Unchanged otherwise.
     * @return True if a byte was successfully retrieved, false otherwise.
     */
    bool Get(uint8_t &byte) {
        if (m_start == m_end) {
            return false;
        }
        byte = m_buffer[m_start++];
        if (m_start == FIFO_SIZE + 1) {
            m_start = 0;
        }
        return true;
    }

private:
    /*
     * The actual queue is one byte longer than required as we can't fill the entire queue.
     */
    uint8_t m_buffer[FIFO_SIZE + 1];

    /*
     * Indicates where payload starts in m_buffer. Range 0 to FIFO_SIZE - 1 inclusive.
     * If m_start == m_end then there is no payload.
     */
    uint16_t m_start;

    /*
     * Indicates where payload ends in m_buffer. Range 0 to FIFO_SIZE - 1 inclusive.
     */
    uint16_t m_end;
};


#endif //VMG_FIFO_H
