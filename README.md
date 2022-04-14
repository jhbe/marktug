# Summary

This is a firmware for a [Matek 405 STD](https://customquads.com.au/collections/3-flight-controller/products/matek-f405-std-flight-controller)
paired with a [Matek GPS + Magnetometer](https://customquads.com.au/products/matek-gps-compass-module-m8q-5884) and two reversible thrusters.



# Channels

Channel 1 (aileron) turns left and right.

Channel 2 (elevator) drives forward and backward.

Channel 3 (throttle) determines the maximum allowed amount of forward thrust. This can be used to reduce the amount of thrust used by the autopilot. This does not affect manual drive.

Channel 4 (rudder) determines the maximum amount of turning thrust. This can be used to reduce the amount of turning thrust used by the autopilot. This does not affect manual drive.

Channel 5 (first AUX channel) determines the autopilot mode:

  Value   | Autopilot Mode
----------|--------------
 < 1250us | Engaged using the current position. Channels 1 and 2 are not used.
   1500us | Disengaged. Manual drive using channels 1 and 2.
\> 1750us | Engaged using the previously used autopilot position (not the current one). Channels 1 and 2 are not used.

# Telemetry

[FlySky telemetry](https://github.com/betaflight/betaflight/wiki/Single-wire-FlySky-%28IBus%29-telemetry):  

Name               | Sensor Id | Description
-------------------|-----------|-----------------------
Current Heading    |      0x08 | The current heading or zero if unavailable.
Course Over Ground |      0x04 | The course to the hold position or zero if the autopilot doesn't have a hold position.
GPS Dist           |      0x14 | The distance to the hold position or 655.35 deg if the autopilot doesn't have a hold position.
GPS Latitude       |      0x80 | The latitude of the hold position or zero if the autopilot doesn't have a hold position.
GPS Longitude      |      0x81 | The longitude of the hold position or zero if the autopilot doesn't have a hold position.

# Toolchain

Download the arm tool chain from [here](https://developer.arm.com/-/media/Files/downloads/gnu-rm/9-2020q2/gcc-arm-none-eabi-9-2020-q2-update-x86_64-linux.tar.bz2)
and extract it to the marktug root.

Download the STM32 F4 Standard Periphiral Libraries from [here](https://www.st.com/en/embedded-software/stsw-stm32065.html)
and extract it to the marktug root.

With the marktug repo as the current directory:

    git clone https://github.com/dmitrystu/libusb_stm32.git
    cd libusb_stm32
    git clone https://github.com/ARM-software/CMSIS_5.git CMSIS
    git clone --recurse-submodules https://github.com/dmitrystu/stm32h.git CMSIS/Device/ST

With marktug/test as your current directory:

    git clone https://github.com/google/googletest.git

Your marktug directory should now have the following sub directories:

    gcc-arm-none-eabi-9-2020-q2-update
    test/googletest
    libusb_stm32
    src
    test
    STM32F4xx_DSP_StdPeriph_Lib_V1.8.0

# Building

    cmake -B cmake-build-debug
    make -C cmake-build-debug

The result is the file cmake-build-debug/src/marktug.axf

# Test

To run the unit tests:

    cmake-build-debug/test/all-test

# Wiring

Pin | Label | Function
----|-------|----------
C06 | S1    | Right Motor
C07 | S2    | Left Motor
A00 | TX4   | Ibus Telemetry
A09 | TX1   | GPS TX
A10 | RX1   | GPS RX
A03 | RX2   | PPM
B06 | SCL   | Compass Clock
B07 | SDA   | Compass Data
A15 | S5    | Debug 0
A08 | S6    | Debug 1

# References

- [F405 Reference manual](https://www.st.com/resource/en/reference_manual/dm00031020-stm32f405415-stm32f407417-stm32f427437-and-stm32f429439-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)

