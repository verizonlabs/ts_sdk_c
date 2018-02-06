[Home](../README.md)


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

### Conventions
- ```ts_[component].[function]``` vs ```ts_[component]_[function]```
- ```ts_[component]_[function]``` vs ```_ts_[component]_[function]```


### Configuration
- explain vtables
- explain tick vs rtos vs os variants

### Toolchain
- explain triplets
- explain toolchain vs platform

### Platform
- show matrix, board x mcu x component + opt. header boards and component

## Use

### Application

### Message

### Platform
- ts_platform vs platforms


## Integrate

![UML Design](images/communication_diagram.png "UML Design Diagram")

### Service

### Transport

### Security

### Connection

#### Type

#### Controller

## Debug

## Examples

- using service w/module-based transport
- using various connection types or controllers

