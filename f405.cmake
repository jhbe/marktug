set(CMAKE_C_FLAGS "-mthumb -mcpu=cortex-m4 -march=armv7e-m -mfloat-abi=hard -mfpu=fpv4-sp-d16 -fsingle-precision-constant -Wdouble-promotion -DUSBD_SOF_DISABLED -DARMCM4 -DCORTEX_M4 -DSTM32F40_41xxx -DHSE_VALUE=8000000 -DUSE_FULL_ASSERT -DSTM32F4XX -DSTM32F405xx -DSTM32F4 -DUSE_STDPERIPH_DRIVER -ffunction-sections -fdata-sections ")
set(CMAKE_CXX_FLAGS ${CMAKE_C_FLAGS})
set(ASM_OPTIONS "${CFLAGS} -x assembler-with-cpp")
set(CMAKE_ASM_FLAGS "${CFLAGS} ${ASM_OPTIONS}")
set(CMAKE_EXE_LINKER_FLAGS "--specs=nosys.specs --specs=nano.specs -u _printf_float -u _scanf_float -Wl,--gc-sections -T ../stm32_flash_f405.ld -Wl,-Map=${CMAKE_PROJECT_NAME}.map")

