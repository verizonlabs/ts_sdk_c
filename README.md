## ThingSpace IoT Device Toolkit

The ThingSpace SDK allows developers to integrate devices into the Verizon ThingSpace IoT platform.

### Quick Start
The SDK has several language bindings including this one, the embedded-C binding. You will need to update the submodules after cloning this repository,...

#### Clone
```
$ git clone https://github.com/verizonlabs/ts_sdk_c.git
$ cd ts_sdk_c
$ git submodule update --init --recursive
```

#### Credentialing

TBD

#### Build

TBD - e.g., CMake is currently the only build configuration system supported. CLion as the primary IDE. To build the framework, simply run 'cmake' followed by 'make', as follows,

```
$ cmake . -B./cmake-build-debug -DTOOLCHAIN=OFF -DCMAKE_BUILD_TYPE=Debug -G "CodeBlocks - Unix Makefiles"
$ cd cmake-build-debug
$ make
 
OR
    
$ cmake . -B./cmake-build-release -DTOOLCHAIN=OFF -DCMAKE_BUILD_TYPE=Release -G "CodeBlocks - Unix Makefiles"
$ cd cmake-build-release
$ make
```

## Directory Structure

The SDK was arranged to allow the user to integrate at various layers, customize various SDK components, and customize hardware integration.

```
sdk                     -- the thingspace client framework 
 
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
    
toolchain               -- optional cross compiler toolchain

documents               -- developer documentation

```

[Documentation](documents/README.md)

