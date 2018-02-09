#!/bin/bash
#cmake . -B./cmake-build-debug \
#	-DCMAKE_BUILD_TYPE=Debug \
#	-DTOOLCHAIN_PREFIX=/Users/Ira/source/embedded/toolchain/gcc-arm-none-eabi-7-2017-q4-major \
#	-DTARGET_TRIPLET=arm-none-eabi \
#	-DSTM32Cube_DIR=/Users/Ira/source/embedded/STM32Cube_FW_F4_V1.19.0 \
#	-DSTM32_CHIP=STM32F401RE \
#	-DCMAKE_TOOLCHAIN_FILE=/Users/Ira/source/embedded/stm32-cmake/cmake/gcc_stm32.cmake

cmake . -B./cmake-build-debug \
	-DBUILD_EXAMPLES=ON \
	-DCMAKE_BUILD_TYPE=Debug \
	-DTOOLCHAIN_PREFIX=/Users/v768213/work/cortexm/gcc-arm-none-eabi-7-2017-q4-major \
	-DTARGET_TRIPLET=arm-none-eabi \
	-DSTM32Cube_DIR=/Users/v768213/work/cortexm/STM32Cube_FW_L4_V1.11.0 \
	-DSTM32_CHIP=STM32L476RG \
	-DCMAKE_TOOLCHAIN_FILE=/Users/v768213/work/stm32-cmake/cmake/gcc_stm32.cmake
