#!/bin/bash
cmake . -B./cmake-build-debug \
	-DCMAKE_BUILD_TYPE=Debug \
	-DTOOLCHAIN_PREFIX=/Users/v585584/Source/embedded/toolchain/gcc-arm-none-eabi-7-2017-q4-major \
	-DTARGET_TRIPLET=arm-none-eabi \
	-DSTM32Cube_DIR=/Users/v585584/Source/embedded/STM32Cube_FW_F4_V1.19.0 \
	-DSTM32_CHIP=STM32F401RE \
	-DCMAKE_TOOLCHAIN_FILE=/Users/v585584/Source/embedded/stm32-cmake/cmake/gcc_stm32.cmake 

