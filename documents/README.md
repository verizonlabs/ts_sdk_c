[Home](../README.md)

## Design

The SDK is designed to allow a developer to quickly prototype an IoT device on their desktop, as well as, implement an hardened application and cross-compile to a target MCU.

The SDK itself is composed of two main parts, the API, (seen in the [sdk](../sdk) directory), and the components (placed in the [sdk_components](../sdk_components) directory).

When creating a new application, the developer must first select a example platform from ./examples/platforms directory, or create their own by implementing the "ts_device" and "ts_platform" interfaces.

Although the SDK can be configured and customized in many various ways, the easiest use is demonstrated in ./examples/applications. There you may notice a simple application that sends sensor data, and acts on actuator commands using the ts_service interface (and ts_message abstraction). 

If you use the configuration as is, then a simple application can be implemented on you MAC or LINUX like the following example (see ./examples/applications/simple/main/c for the complete code listing),
```
int main() {

    // initialize the platoform (implemented per platform)
    ts_platform_initialize();

    // initialize sensor cache (its contents would come from the actual hardware)
    TsMessage_t sensor;
    ts_message_create( &sensor );
    ts_message_set_float( sensor, "temperature", 50.5 );
    
    // initialize the service
    TsServiceRef_t service;
    ts_service_create( &service );
    
    // register a message handler
    ts_service_dequeue( service, TsServiceActionMaskAll, handler );
    
    // connect to the cloud
    ts_service_dial( service, address );
    
    // start a single-threaded run-loop
    while( running ) {

        // send telemetry at some interval,...
        ts_service_enqueue( service, sensor );
        
        // pass control to the SDK for a period of time,...
        ts_service_tick( service, 5 * TS_TIME_SEC_TO_USEC );    
    }
    
    // clean-up
    ts_service_disconnect( service );
    ts_service_destroy( service );
}

// my message handler
static TsStatus_t handler( TsServiceRef_t service, TsServiceAction_t action, TsMessageRef_t message ) {

    // ... act on action and message ...
    return TsStatusOk;
}

```

### Components

The SDK is designed according to the following UML diagram, and it can be customized at any point in the diagram where the stereotype, "vtable" is indicated. 

![UML Design](images/communication_diagram.png "UML Design Diagram")

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

#### Registration and Credentialing

TBD

#### Platforms

TBD

#### Applications

TBD