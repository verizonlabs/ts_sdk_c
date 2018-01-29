


```
security   - encryption, decryption and credentialing.
controller - access to the network layer (e.g., at-commands).
driver     - access to the communications device.
```



```

platforms or applications

    none
    
        nucleo
                link.ld
                ts_pins.c
                ts_platform.c
                ts_driver_port.c
                
        freedom
                link.ld
                ts_pins.c
                ts_platform.c
                ts_driver_port.c

    freeRTOS
    
    threadX
        renesas-s5-d9
                link.ld
                ts_pins.c
                ts_platform.c
                ts_driver_device.c
                ts_driver_port.c
                
    unix
        raspberry-pi
                ts_platform.c
                ts_driver_device.c
                
        ts_platform.c
            
```
            
