# Copyright (C) 2017, Verizon, Inc. All rights reserved.
include( CMakeForceCompiler )

# enable toolchain settings, the default is OFF
# however, the TOOLCHAIN option can be overridden from the command line,
# e.g., cmake /source -B/source/cmake-build-debug -DTOOLCHAIN=ON -DCMAKE_BUILD_TYPE=Debug
option( TOOLCHAIN "Compile using toolchain settings" OFF )
message( STATUS "## Toolchain                    (ON|OFF) : ${TOOLCHAIN}" )

if( TOOLCHAIN )

    # build using toolchain
    set( CMAKE_SYSTEM_NAME Generic )
    set( CMAKE_SYSTEM_VERSION 1 )
    set( ROOTPATH   "/toolchain" )

    # compiler "triplet" prefix
    # * "arm-linux-gnueabihf"  raspberry-pi
    # * "arm-none-eabi"        cortex-R and M mcus, bare-metal targets (e.g., ST32)
    # * "arm-eabi"             cortex-A mcus, bare-metal targets
    set( TRIPLET    "arm-linux-gnueabihf" CACHE STRING "Triplet prefix: arm-linux-gnueabihf, arm-none-eabi or arm-eabi" FORCE)
    message( STATUS "## Triplet                               : ${TRIPLET}" )

    # compiler and other tools
    # note, some reverse the order, e.g., gcc-${TRIPLET} vs ${TRIPLET}-gcc
    set( GCC        "${TRIPLET}-gcc" )
    set( GPP        "${TRIPLET}-g++" )
    set( GDB        "${TRIPLET}-gdb" )
    set( BINUTILS   "${TRIPLET}-binutils" )
    set( OBJCOPY    "${TRIPLET}-objcopy" )

    # force cmake objcopy
    set( CMAKE_OBJCOPY ${ROOTPATH}/${TRIPLET}/bin/${OBJCOPY} CACHE FILEPATH "The toolchain objcopy command " FORCE )

    # specify the cross compiler.
    # note, this is deprecated, yet it works correctly - setting the cmake variable alone doesnt work.
    set( CMAKE_FIND_ROOT_PATH ${ROOTPATH}/${TRIPLET}/${TRIPLET} )
    cmake_force_c_compiler( ${ROOTPATH}/${TRIPLET}/bin/${GCC} GNU )
    cmake_force_cxx_compiler( ${ROOTPATH}/${TRIPLET}/bin/${GPP} GNU )

    # common compiler flags
    # -v                    Add if needing verbose output
    # -f no-builtin         Don’t recognize built-in functions that do not begin with ‘__builtin_’ as prefix.
    # -f function-sections  Place each function or data item into its own section in the output file if the target
    #                       supports arbitrary sections.
    # -f data-sections      See function-sections
    # -f no-strict-aliasing Allow dereferencing a pointer that aliases another of an incompatible type.
    # -m thumb              An ARM option, force code generation in the Thumb (T16/T32) ISA, depending on the architecture level.
    # -m fpu                An ARM option, specifies the fpu for which to tune the performance of this function.
    # -m cpu                An ARM option, specifies the name of the target ARM processor.
    # -m arch               An ARM option, specifies the name of the target ARM architecture. Currently not specified.
    # -m float-abi          An ARM option, specifies which floating-point ABI to use. ‘hard’ allows generation of
    #                       floating-point instructions and uses FPU-specific calling conventions.
    # TODO - missing "-f lto" (link time optimization)? e.g., optimizing static variable use - expensive compile time
    set( COMMON_FLAGS "-Werror -Os -fno-builtin -ffunction-sections -fdata-sections -fno-strict-aliasing" )
    set( COMMON_FLAGS "${COMMON_FLAGS} -mthumb -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16" )

    # (TODO) remove hack, the __GLIBC__ should be defined automatically - we should be linking glibc (not just libc)?
    set( CMAKE_C_FLAGS "${COMMON_FLAGS} -D __GLIBC__ -std=c99" )
    message( STATUS "## CMAKE_C_FLAGS                         : ${CMAKE_C_FLAGS}" )

    set( CMAKE_CXX_FLAGS "${COMMON_FLAGS} -std=c++03" )
    message( STATUS "## CMAKE_CXX_FLAGS                       : ${CMAKE_CXX_FLAGS}" )

    # bare metal linker flags
    # --gc-sections Enable garbage collection of unused input sections.
    # -nostdlib     Only search library directories explicitly specified on the command line.
    # -static       Do not link against shared libraries.
    # -nostartfiles Do not use the standard system startup files when linking.
    # (TODO) arm-none-eabi/bin/ld: warning: cannot find entry symbol _start; defaulting to 00008000
    if( ${TRIPLET} STREQUAL "arm-none-eabi" OR ${TRIPLET} STREQUAL "arm-eabi" )
        set( CMAKE_EXE_LINKER_FLAGS "-Wl,--gc-sections --specs=nosys.specs -nostdlib -static -nostartfiles" )
    endif()

    # other recommended settings
    set( CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER )
    set( CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY )
    set( CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY )
    set( CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY )

else()

    # build using native
    set( CMAKE_C_STANDARD 99 )
    set( CMAKE_MACOSX_RPATH 0 )

    cmake_force_c_compiler( /usr/bin/gcc GNU )
    cmake_force_cxx_compiler( /usr/bin/g++ GNU )

endif()
message( STATUS "## C-Compiler                            : ${CMAKE_C_COMPILER}" )
message( STATUS "## CPP-Compiler                          : ${CMAKE_CXX_COMPILER}" )
