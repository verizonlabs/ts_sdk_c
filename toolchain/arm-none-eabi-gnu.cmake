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

if (WIN32)
	set( TOOLCHAIN_EXEC_SUFFIX ".exe" )
else()
	set( TOOLCHAIN_EXEC_SUFFIX "" )
endif()

set( GCC "${TARGET_TRIPLET}-gcc${TOOLCHAIN_EXEC_SUFFIX}" )
set( GPP "${TARGET_TRIPLET}-g++${TOOLCHAIN_EXEC_SUFFIX}" )
set( GDB "${TARGET_TRIPLET}-gdb${TOOLCHAIN_EXEC_SUFFIX}" )
set( OBJCOPY "${TARGET_TRIPLET}-objcopy${TOOLCHAIN_EXEC_SUFFIX}" )
set( OBJDUMP "${TARGET_TRIPLET}-objdump${TOOLCHAIN_EXEC_SUFFIX}" )
set( AR "${TARGET_TRIPLET}-ar${TOOLCHAIN_EXEC_SUFFIX}" )
set( RANLIB "${TARGET_TRIPLET}-ranlib${TOOLCHAIN_EXEC_SUFFIX}" )
set( SIZE "${TARGET_TRIPLET}-size${TOOLCHAIN_EXEC_SUFFIX}" )
set( CPPFILT "${TARGET_TRIPLET}-c++filt${TOOLCHAIN_EXEC_SUFFIX}" )

set( CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY )
set( CMAKE_C_COMPILER "${TOOLCHAIN_BIN_DIR}/${GCC}" CACHE INTERNAL "c compiler" )
set( CMAKE_CXX_COMPILER "${TOOLCHAIN_BIN_DIR}/${GPP}" CACHE INTERNAL "c++ compiler" )
set( CMAKE_ASM_COMPILER "${TOOLCHAIN_BIN_DIR}/${GCC}" CACHE INTERNAL "assembler" )
set( CMAKE_AR "${TOOLCHAIN_BIN_DIR}/${AR}" CACHE INTERNAL "archive tool" )
set( CMAKE_RANLIB "${TOOLCHAIN_BIN_DIR}/${RANLIB}" CACHE INTERNAL "archive indexer" )
set( CMAKE_OBJCOPY "${TOOLCHAIN_BIN_DIR}/${OBJCOPY}" CACHE INTERNAL "objcopy tool" )
set( CMAKE_OBJDUMP "${TOOLCHAIN_BIN_DIR}/${OBJDUMP}" CACHE INTERNAL "objdump tool" )
set( CMAKE_SIZE "${TOOLCHAIN_BIN_DIR}/${SIZE}" CACHE INTERNAL "size tool" )
set( CMAKE_CPPFILT "${TOOLCHAIN_BIN_DIR}/${CPPFILT}" CACHE INTERNAL "c++filt" )

execute_process( COMMAND ${CMAKE_C_COMPILER} --print-file-name=liblto_plugin.so OUTPUT_VARIABLE LTO_PLUGIN OUTPUT_STRIP_TRAILING_WHITESPACE )

# In order to use Link Time Optimization (LTO), we need to use a GCC plugin. Currently, LTO works with GCC v4.9.3 and not with GCC v7.2.1 at
# least for the embedded toolchain case.
if( CMAKE_BUILD_TYPE STREQUAL "Release" )
	set( CMAKE_C_ARCHIVE_CREATE "<CMAKE_AR> qc --plugin ${LTO_PLUGIN} <TARGET> <LINK_FLAGS> <OBJECTS>" )
	set( CMAKE_C_ARCHIVE_FINISH "<CMAKE_RANLIB> --plugin ${LTO_PLUGIN} <TARGET>" )
endif()

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

# Make sure to remove unused functions and unused data.
# nano.specs and nosys.specs links to the Newlib-nano library and eliminates dependencies on system functions
# typically not present on embedded systems. This includes routines such as exit, write, sbrk etc.
# Also enable floats to work with the (s|v|sn)*printf family of functions.
set( COMMON_LINKER_FLAGS "-Wl,--gc-sections -Wl,--as-needed --specs=nosys.specs --specs=nano.specs -u _printf_float" CACHE INTERNAL "common linker flags" )

set( CMAKE_C_FLAGS "${COMMON_COMPILER_FLAGS}" CACHE INTERNAL "c compiler flags" )
set( CMAKE_CXX_FLAGS "${COMMON_COMPILER_FLAGS}" CACHE INTERNAL "c++ compiler flags" )
set( CMAKE_ASM_FLAGS "${ARCHFLAGS}" CACHE INTERNAL "assembler flags" )
set( CMAKE_EXE_LINKER_FLAGS "${COMMON_LINKER_FLAGS}" CACHE INTERNAL "linker flags" )

set( CMAKE_C_FLAGS_DEBUG "-std=c99 -O0 -g3" CACHE INTERNAL "c compiler debug flags" )
set( CMAKE_CXX_FLAGS_DEBUG "-std=c++11 -O0 -g3" CACHE INTERNAL "c++ compiler debug flags" )
set( CMAKE_ASM_FLAGS_DEBUG "-g3" CACHE INTERNAL "assembler debug flags" )
set( CMAKE_EXE_LINKER_FLAGS_DEBUG "" CACHE INTERNAL "linker debug flags" )

set( CMAKE_C_FLAGS_RELEASE "-std=gnu11 -Os -flto" CACHE INTERNAL "c compiler release flags" )
set( CMAKE_CXX_FLAGS_RELEASE "-std=c++11 -O2 -flto" CACHE INTERNAL "c++ compiler release flags" )
set( CMAKE_ASM_FLAGS_RELEASE "" CACHE INTERNAL "assembler release flags" )
set( CMAKE_EXE_LINKER_FLAGS_RELEASE "-Os" CACHE INTERNAL "linker release flags" )

set( CMAKE_FIND_ROOT_PATH ${TOOLCHAIN_PREFIX}/${TARGET_TRIPLET} )
set( CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER )
set( CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY )
set( CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY )
