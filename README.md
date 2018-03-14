## ThingSpace IoT Device Toolkit

The ThingSpace SDK allows developers to integrate devices into the Verizon ThingSpace IoT platform.

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
$ cmake . -B./cmake-build-debug -DCMAKE_BUILD_TYPE=Debug -DCMAKE_MAKE_PROGRAM=gmake -G "CodeBlocks - Unix Makefiles"
$ cd cmake-build-debug
$ make
```

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
$ cmake . -B./cmake-build-debug -DCMAKE_TOOLCHAIN_FILE=./tools/arm-none-eabi-gnu.cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_MAKE_PROGRAM=gmake -G "CodeBlocks - Unix Makefiles"
$ cd cmake-build-debug
$ make
```

## Examples

#### Registration and Credentialing

TBD

#### Platforms

TBD

#### Applications

TBD

