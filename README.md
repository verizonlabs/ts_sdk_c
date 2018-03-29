## ThingSpace IoT Device Toolkit

The ThingSpace SDK allows developers to integrate devices into the Verizon ThingSpace IoT platform.

### Design

See [documents](./documents/README.md) for design documentation and links.

### Quick Start

The TS-SDK has several language bindings including this one, the embedded-C binding. Once you've cloned the repo, you will also need to update its submodules (see below). 

```
$ git clone https://github.com/verizonlabs/ts_sdk_c.git
$ cd ts_sdk_c
```

Some submodules are held by private repositories, and will only be downloaded if you have permissions to access those repositories.

#### Build: Native

If you wish to build the TS-SDK library on a system where you've cloned the source, then simply install the prerequisites and run cmake without a toolchain, then gmake to compile.

##### Prerequisites

1. cmake >= 3.7
2. gcc >= 4.2.1
3. (optional) clion >= 2017.3.1

##### Command Line

From the cloned TS-SDK directory, run 'cmake .' with the following parameters,

```
-B [./cmake-build-debug | ./cmake-build-release]
-DCMAKE_BUILD_TYPE      = [Debug | Release (default)]
-DBUILD_EXAMPLES        = [ON | OFF (default)]
```

Followed by 'make' in the newly created build directory. For example,

```
$ cmake . -B./cmake-build-debug -DCMAKE_BUILD_TYPE=Debug -DCMAKE_MAKE_PROGRAM=make -G "CodeBlocks - Unix Makefiles"
$ cd cmake-build-debug
$ make
```

If you change the cmake parameters to do a different build, you will have to move or delete
the existing cmake-build-debug or cmake-build-release directory before running cmake again.

#### Build: Cross-compile 

##### Prerequisites 

1. cmake >= 3.7
2. [Arm Cortex-M Toolchain](https://developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads) >= 7-2017-q4-major

##### Command Line

Run 'cmake .' with the following parameters,

```
-B [./cmake-build-debug | ./cmake-build-release]
-DCMAKE_BUILD_TYPE      = [Debug | Release (default)]
-DBUILD_EXAMPLES        = [ON | OFF (default)]
-DCMAKE_TOOLCHAIN_FILE  =./cmake/arm-none-eabi-gnu.cmake
-DTOOLCHAIN_PREFIX      = [path to compiler triplet]
```

Followed by 'make' in the newly created build directory. For example,

```
$ cmake . -B./cmake-build-debug -DCMAKE_TOOLCHAIN_FILE=./tools/arm-none-eabi-gnu.cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_MAKE_PROGRAM=make -G "CodeBlocks - Unix Makefiles"
$ cmake . -B./cmake-build-debug \
    -DCMAKE_TOOLCHAIN_FILE=./tools/arm-none-eabi-gnu.cmake \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_MAKE_PROGRAM=make \
    -DTOOLCHAIN_PREFIX=$MY_TOOLCHAIN_DIR/gcc-arm-none-eabi-7-2017-q4-major 
    -G "CodeBlocks - Unix Makefiles"
$ cd cmake-build-debug
$ make
```

On windows,

```$xslt
cmake.exe . -B./cmake-build-debug \
    -DCMAKE_TOOLCHAIN_FILE=./toolchain/arm-none-eabi-gnu.cmake \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_MAKE_PROGRAM=make \
    -DTOOLCHAIN_PREFIX=/c/Program\ Files\ \(x86\)/GNU\ Tools\ ARM\ Embedded/7\ 2017-q4-major/ \
    -G "Unix Makefiles" 
```

If you change the cmake parameters to do a different build, you will have to move or delete
the existing cmake-build-debug or cmake-build-release directory before running cmake again.

### Directory Structure

The SDK was arranged to allow the user to integrate at various layers, customize various SDK components, and customize hardware integration.

```
sdk                     -- the thingspace client framework 
    include             -- the API description
    source              -- core API implementation
 
sdk_components          -- framework options
    service             -- application integration and ThingSpace protocols
    transport           -- pub-sub or point-to-point protocols (e.g., mqtt)
    connection          -- network connectivity  
    security            -- optional connection credentials and security (e.g., ssl)
    controller          -- optional connection modem controllers (e.g., qualcom)
    driver              -- optional connection device driver 
 
sdk_dependencies        -- external vendor libraries
 
examples
    platforms           -- os and hardware specific libraries 
    applications        -- end-user applications (e.g., track-and-trace)
    tests               -- unit tests
    
tools                   -- optional cross compiler toolchain

documents               -- developer documentation
```


