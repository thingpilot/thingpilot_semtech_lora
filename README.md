# Thing Pilot lorawan Release Notes
**v0.2.0** *29/11/2019*
 - To be used with SX1276 (or compatible) && STM32L0xx
 - Operating in Class C and Class A
 - Bug fix (Hanging-downlink)
 - Accepts Downlinks only in a hex format, returns a decimal value
 - The following ports should never be used from the user:
 - Port 220 reserved for scheduling a clock synch(HEX 2 bytes)
 - Port 221 reserved for Synching the time (HEX - 4 bytes)
 - Port 222 reserved for Setting the scheduler (HEX -N(even)bytes)
 - Port 223 reserved for Resetting the device

 - For non-RTOS systems
**v0.1.0** *31/10/2019*
 -To be used with SX1276 (or compatible) && STM32L0xx
 - Operating in Class C
 - Downlink only after an uplink
 - Accepts Downlinks only in a hex format, returns a decimal value
 - For non-RTOS systems

