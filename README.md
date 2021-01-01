Based on the great work over at \
https://github.com/ccutrer/balboa_worldwide_app/wiki
https://github.com/ccutrer/balboa_worldwide_app/blob/master/doc/protocol.md


# esp8266_spa
Control for a Balboa spa controller using the esp8266 (tested on BP2100 and BP601 series)

The sketch connects to the tub, gets an ID and spits out the state to the MQTT broker on topics "Spa/#".
You can control the tub using the subscribed topics, f.e. "Spa/light" with message "ON" for maximum compatability with openhab.

Maybe you need to adjust the sketch to your tub configuration (number of pumps, connection of blower, nr of lights...). You may find the DEBUG comments useful for this task.

Bonus: you may add several relays or such like I did ;-)

# Getting started
- Make sure your Spa is using a Balboa controller
- This is known to work on and developed on a BP2100G0 and BP601 series controllers. Yours may be compatible.
- Get the parts
- Flash the esp8266 fom the Arduino IDE or PlatformIO
- Connect everything together
- Hook up on the Spa
- Enjoy and get tubbin' ;-)

# Parts
- Get a esp8266, perferable a Wemos D1 Pro in case you need to attach a seperate antenna
- RS485 bus transceiver, (for example the "ARCELI TTL To RS485 Adapter 485 Serial Port UART Level Converter Module 3.3V 5V")
- A DC-DC converter for powering from the Tub (MT3608 for example)
- breadboard, wire etc...


# Hardware connections
![Example](https://github.com/EmmanuelLM/esp8266_spa/blob/master/esp8266_spa_bb.png)
- Look up finding the right wires on https://github.com/ccutrer/balboa_worldwide_app/wiki#physical-layer
- Connect the DC-DC converter to the supply wires (+ and Ground) from the Tub
- Set the DC-DC converter to output 3.3V. This output voltage (+ Ground) should then connect to the Wemos D1 Mini Pro and the RS485 transceiver
- Connect the RS485 transceiver to the A and B wires
- Connect the esp8266-TX to the RS485 TX
- Connect the esp8266-RX to the RS485 RX
![Example] (https://github.com/EmmanuelLM/esp8266_spa/blob/master/PXL_20210101_104120166.jpg)

# Debug
- First, the RX (and to some extent the TX) LEDs of the RS485 transceiver (if using the one above) should light up as data goes through. If that is the case you know data is being converted from RS485 to TTL
- The Wemos D1 Mini Pro should spit out data to the MQTT broker (MQTT Spy can be useful here to see: 1. that it is connected to the wifi; 2. that it is connected to the broker). If that is the case, you know the device can communicate over MQTT

# Appetiser using OpenHab
![Example](https://github.com/cribskip/esp8266_spa/blob/master/spa_openhab.png)

# TODO
- Add more documentation
- Add fault reporting
- Add more setting possibilities (filter cycles, preferences maybe)
