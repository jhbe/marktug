cmake_minimum_required(VERSION 3.13)
project(marktug C CXX ASM)

set(CMAKE_CX X_STANDARD 14)

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(REPO ${CMAKE_CURRENT_LIST_DIR})
set(TOOLS_PREFIX ${REPO}/../gcc-arm-none-eabi-9-2020-q2-update/bin/arm-none-eabi-)

# @formatter:off
set(CMAKE_C_COMPILER   ${TOOLS_PREFIX}gcc)
set(CMAKE_CXX_COMPILER ${TOOLS_PREFIX}g++)
set(CMAKE_LINKER       ${TOOLS_PREFIX}ld)
set(CMAKE_ASM_COMPILER ${TOOLS_PREFIX}gcc)
set(CMAKE_OBJCOPY      ${TOOLS_PREFIX}objcopy)
set(CMAKE_STRIP        ${TOOLS_PREFIX}strip)
set(CMAKE_CPP          ${TOOLS_PREFIX}cpp)
# @formatter:on

set(CMAKE_FIND_ROOT_PATH ${REPO}/gcc-arm-none-eabi-9-2020-q2-update/lib/gcc/arm-none-eabi/9.3.1)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

set(STDPERIPH_LIBRARIES ${REPO}/../STM32F4xx_DSP_StdPeriph_Lib_V1.9.0/Libraries)
set(USB_STM32_LIBRARIES ${REPO}/../libusb_stm32)

# @formatter:off
set(CMSIS_INC     ${STDPERIPH_LIBRARIES}/CMSIS/Include)
set(DEVICE_INC    ${STDPERIPH_LIBRARIES}/CMSIS/Device/ST/STM32F4xx/Include)
set(STDPERIPH_INC ${STDPERIPH_LIBRARIES}/STM32F4xx_StdPeriph_Driver/inc)
set(STDPERIPH_SRC ${STDPERIPH_LIBRARIES}/STM32F4xx_StdPeriph_Driver/src)
set(LIB_CORE_INC  ${USB_STM32_LIBRARIES}/CMSIS/CMSIS/Core/Include)
set(LIB_DEV_INC   ${USB_STM32_LIBRARIES}/CMSIS/Device/ST)
set(LIB_USB_INC   ${USB_STM32_LIBRARIES}/inc)
set(LIB_USB_SRC   ${USB_STM32_LIBRARIES}/src)
# @formatter:on

set(CMAKE_C_FLAGS "-mthumb -mcpu=cortex-m4 -march=armv7e-m -mfloat-abi=hard -mfpu=fpv4-sp-d16 -fsingle-precision-constant -Wdouble-promotion -DUSBD_SOF_DISABLED -DARMCM4 -DCORTEX_M4 -DSTM32F40_41xxx -DHSE_VALUE=8000000 -DUSE_FULL_ASSERT -DSTM32F4XX -DSTM32F405xx -DSTM32F4 -DUSE_STDPERIPH_DRIVER -ffunction-sections -fdata-sections ")
set(CMAKE_CXX_FLAGS ${CMAKE_C_FLAGS})
set(ASM_OPTIONS "${CFLAGS} -x assembler-with-cpp")
set(CMAKE_ASM_FLAGS "${CFLAGS} ${ASM_OPTIONS}")
set(CMAKE_EXE_LINKER_FLAGS "--specs=nosys.specs --specs=nano.specs -u _printf_float -u _scanf_float -Wl,--gc-sections -T ../stm32_flash_f405.ld -Wl,-Map=${CMAKE_PROJECT_NAME}.map")

add_executable(marktug.axf
        autopilot.cpp
        compass.cpp
        debug.cpp
        extcount.cpp
        gps.cpp
        i2c_bitbang.cpp
        ibus.cpp
        latlong.cpp
        marktug.cpp
        qmc5883.cpp
        ppm.cpp
        motors.cpp
        usart.cpp
        led.c
        stdbulkusb.c
        startup_stm32f40_41xxx.s
        system_stm32f4xx.c
        ${LIB_USB_SRC}/usbd_core.c
        ${LIB_USB_SRC}/usbd_stm32f429_otgfs.c
        ${STDPERIPH_SRC}/misc.c
        ${STDPERIPH_SRC}/stm32f4xx_adc.c
        ${STDPERIPH_SRC}/stm32f4xx_i2c.c
        ${STDPERIPH_SRC}/stm32f4xx_usart.c
        ${STDPERIPH_SRC}/stm32f4xx_gpio.c
        ${STDPERIPH_SRC}/stm32f4xx_rcc.c
        ${STDPERIPH_SRC}/stm32f4xx_spi.c
        ${STDPERIPH_SRC}/stm32f4xx_tim.c)

target_include_directories(marktug.axf PUBLIC
        .
        ${DEVICE_INC}
        ${STDPERIPH_INC}
        ${LIB_USB_INC}
        ${LIB_CORE_INC}
        ${LIB_DEV_INC})
