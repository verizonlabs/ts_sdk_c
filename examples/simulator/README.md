## Simple Application Simulator Docker Container

### Build

After cloning the ts_sdk_c repo and updating all of the submodules you can build the simulator by performing the following commands:
```$xslt
home      $ cd ts_sdk_c/examples/simulator
simulator $ ./build.bash
```

### Simulate

After building the simulator, perform the following commands:
```$xslt
simulator $ ./simulate.bash [host:port] [[ca-host]
```

Where the optional "host:port" parameter is the MQTT server proxy address and port. If the host is different than the server certificate, then you may pass the ca-host parameter for sake ca-cert host validation.
