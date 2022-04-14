cmake_minimum_required(VERSION 3.13)
project(marktug-tests)

add_subdirectory(googletest)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR} ${gmock_SOURCE_DIR}/include ${gmock_SOURCE_DIR})

add_executable(all-test
        autopilot-test.cpp
        fifo-test.cpp
        gps-test.cpp
        latlong-test.cpp
        ppm-test.cpp
        ../src/autopilot.cpp
        ../src/gps.cpp
        ../src/latlong.cpp
        ../src/ppm.cpp)

target_include_directories(all-test PUBLIC ../src)
target_link_libraries(all-test gtest gmock gtest_main)
