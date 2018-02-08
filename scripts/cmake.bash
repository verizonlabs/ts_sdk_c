#!/bin/bash
cmake . -B./cmake-build-debug \
	-DCMAKE_BUILD_TYPE=Debug \
	-DTOOLCHAIN_PREFIX=/Users/Ira/source/embedded/toolchain/gcc-arm-none-eabi-7-2017-q4-major \
	-DTARGET_TRIPLET=arm-none-eabi \
	-DSTM32Cube_DIR=/Users/Ira/source/embedded/STM32Cube_FW_F4_V1.19.0 \
	-DSTM32_CHIP=STM32F401RE \
	-DCMAKE_TOOLCHAIN_FILE=/Users/Ira/source/embedded/stm32-cmake/cmake/gcc_stm32.cmake 

