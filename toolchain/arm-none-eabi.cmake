# Copyright (c) 2018 Verizon Inc. All rights reserved.
# Toolchain file for the GCC ARM compiler suite
cmake_minimum_required( VERSION 3.7 )
set( CMAKE_SYSTEM_NAME Generic )
set( CMAKE_SYSTEM_PROCESSOR ARM )

FILE( TO_CMAKE_PATH "${TOOLCHAIN_PREFIX}" TOOLCHAIN_PREFIX )

set( TARGET_TRIPLET "arm-none-eabi" )

set( TOOLCHAIN_BIN_DIR "${TOOLCHAIN_PREFIX}/bin" )
set( TOOLCHAIN_INC_DIR "${TOOLCHAIN_PREFIX}/${TARGET_TRIPLET}/include" )
set( TOOLCHAIN_LIB_DIR "${TOOLCHAIN_PREFIX}/${TARGET_TRIPLET}/lib" )

set( GCC "${TARGET_TRIPLET}-gcc" )
set( GPP "${TARGET_TRIPLET}-g++" )
set( GDB "${TARGET_TRIPLET}-gdb" )
set( AS  "${TARGET_TRIPLET}-as" )
set( OBJCOPY "${TARGET_TRIPLET}-objcopy" )
set( OBJDUMP "${TARGET_TRIPLET}-objdump" )
set( SIZE "${TARGET_TRIPLET}-size" )
set( CPPFILT "${TARGET_TRIPLET}-c++filt" )

set( CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY )
set( CMAKE_C_COMPILER "${TOOLCHAIN_BIN_DIR}/${GCC}" CACHE INTERNAL "c compiler" )
set( CMAKE_CXX_COMPILER "${TOOLCHAIN_BIN_DIR}/${GPP}" CACHE INTERNAL "c++ compiler" )
set( CMAKE_ASM_COMPILER "${TOOLCHAIN_BIN_DIR}/${AS}" CACHE INTERNAL "assembler" )
set( CMAKE_OBJCOPY "${TOOLCHAIN_BIN_DIR}/${OBJCOPY}" CACHE INTERNAL "objcopy tool" )
set( CMAKE_OBJDUMP "${TOOLCHAIN_BIN_DIR}/${OBJDUMP}" CACHE INTERNAL "objdump tool" )
set( CMAKE_SIZE "${TOOLCHAIN_BIN_DIR}/${SIZE}" CACHE INTERNAL "size tool" )
set( CMAKE_CPPFILT "${TOOLCHAIN_BIN_DIR}/${CPPFILT}" CACHE INTERNAL "c++filt" )

# By default, turn on the floating point unit hardware. Most Cortex-M4s seem to have it.
# Make sure that the SDK and the example application have the same floating point unit type.
# If they don't, there might be linker issues.
option( FP_HARD "Use floating point unit hardware" ON )
if (FP_HARD)
	set( FP_FLAG "-mfloat-abi=hard" )
else()
	set( FP_FLAG "-mfloat-abi=softfp" )
endif()

# Select a CPU type
set( CPU_TYPE_FLAG "-mcpu=cortex-m4" )
set( CPU_TYPE "cortexm4" )
if( NOT DEFINED CPU_TYPE )
	set( CPU_TYPE "cortexm4" )
	message( STATUS "No CPU specified. Defaulting to cortexm4" )
else()
	STRING( TOLOWER ${CPU_TYPE} CPU_TYPE_LOWER )
	if( CPU_TYPE_LOWER STREQUAL "cortexm4" )
		set( CPU_TYPE_FLAG "-mcpu=cortex-m4" )
	elseif( CPU_TYPE_LOWER STREQUAL "cortexm0" )
		set( CPU_TYPE_FLAG "-mcpu=cortex-m0" )
	else()
		message( FATAL_ERROR "CPU type not supported (${CPU_TYPE})" )
	endif()
endif()

set( ARCHFLAGS "-mthumb ${CPU_TYPE_FLAG} ${FP_FLAG} -mfpu=fpv4-sp-d16" CACHE INTERNAL "Architecture flags" )
# TODO: Do we want to set "-Wcast-align" here?
set( COMMON_COMPILER_FLAGS "${ARCHFLAGS} -Wall -fdata-sections -ffunction-sections" CACHE INTERNAL "Common compiler flags" )

# nano.specs and nosys.specs links to the Newlib-nano library and eliminates dependencies on system functions
# typically not present on embedded systems. This includes routines such as exit, write, sbrk etc.
# Also enable floats to work with the (s|v|sn)*printf family of functions.
set( COMMON_LINKER_FLAGS "-Wl,--gc-sections -Wl,--as-needed --specs=nosys.specs --specs=nano.specs -u _printf_float" CACHE INTERNAL "common linker flags" )

set( CMAKE_C_FLAGS "${COMMON_COMPILER_FLAGS}" CACHE INTERNAL "c compiler flags" )
set( CMAKE_CXX_FLAGS "${COMMON_COMPILER_FLAGS}" CACHE INTERNAL "c++ compiler flags" )
set( CMAKE_ASM_FLAGS "${ARCHFLAGS}" CACHE INTERNAL "assembler flags" )
set( CMAKE_EXE_LINKER_FLAGS "${COMMON_LINKER_FLAGS}" CACHE INTERNAL "linker flags" )

set( CMAKE_C_FLAGS_DEBUG "-std=c99 -O0 -g" CACHE INTERNAL "c compiler debug flags" )
set( CMAKE_CXX_FLAGS_DEBUG "-std=c++11 -O0 -g" CACHE INTERNAL "c++ compiler debug flags" )
set( CMAKE_ASM_FLAGS_DEBUG "-g" CACHE INTERNAL "assembler debug flags" )
set( CMAKE_EXE_LINKER_FLAGS_DEBUG "" CACHE INTERNAL "linker debug flags" )

set( CMAKE_C_FLAGS_RELEASE "-std=c99 -O2 -flto" CACHE INTERNAL "c compiler release flags" )
set( CMAKE_CXX_FLAGS_RELEASE "-std=c++11 -O2 -flto" CACHE INTERNAL "c++ compiler release flags" )
set( CMAKE_ASM_FLAGS_RELEASE "" CACHE INTERNAL "assembler release flags" )
set( CMAKE_EXE_LINKER_FLAGS_RELEASE "-flto" CACHE INTERNAL "linker release flags" )

set( CMAKE_FIND_ROOT_PATH ${TOOLCHAIN_PREFIX}/${TARGET_TRIPLET} )
set( CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER )
set( CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY )
set( CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY )
