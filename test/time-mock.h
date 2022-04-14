#ifndef MARKTUG_TESTS_TIME_MOCK_H
#define MARKTUG_TESTS_TIME_MOCK_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "itime.h"

class TimeMock : public Time {
public:
    MOCK_METHOD(uint64_t, Now, ());
    MOCK_METHOD(void, DelayMs, (uint64_t));
};

#endif
