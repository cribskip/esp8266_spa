Based on the great work over at https://github.com/ccutrer/balboa_worldwide_app/wiki, https://github.com/ccutrer/balboa_worldwide_app/blob/master/doc/protocol.md


# esp8266_spa
Control for a Balboa BP2100G0 spa controller using the esp8266

I'm using a Wemos D1 Mini pro here in conjunction with a cheapo TTL-RS485 tranceiver.
Serial is wired to RE and DI respectively and D1 is 485_enable.

The sketch connects to the tub, gets an ID and spits out the state to the MQTT broker on topics "Spa/#".
You can control the tub using the subscribed topics, f.e. "Spa/light" with message "ON" for maximum compatability with openhab.

Maybe you need to adjust the sketch to your tub configuration (number of pumps, connection of blower, nr of lights...). You may find the DEBUG comments useful for this task.

Bonus: you may add several relays or such like I did ;-)

Enjoy.

# Getting started
- Make sure your Spa is using a Balboa controller
- This is known to work on and developed on a BP2100G0 controller. Yours may be compatible.
- Get the parts
- Flash the esp8266 fom the Arduino IDE
- Connect everything together
- Hook up on the Spa
- Enjoy and get tubbin' ;-)

# Parts
- Get a esp8266, perferable a Wemos D1 Pro in case you need to attach a seperate antenna.
- RS485 bus transceiver, a MAX485 breakout board is sufficient
- A DC-DC converter for powering from the Tub (MT3608 for example)
- Some wire

# Hardware connections
![Example](https://github.com/cribskip/esp8266_spa/blob/master/esp8266_spa.jpg)
- Look up finding the right wires on https://github.com/ccutrer/balboa_worldwide_app/wiki#physical-layer
- Connect the DC-DC converter to the supply wires (+ and Ground) from the Tub
- Connect the RS485 transceiver to the A and B wires
- Connect the esp8266-TX to the rS485 transceiver on pin DI
- Connect the esp8266-RX to the rS485 transceiver on pin RO
- Connect the esp8266-D1 to the rS485 transceiver on both pins RE and DE.

# TODO
- Add more documentation
- Add fault reporting
- Add more setting possibilities (filter cycles, preferences maybe)
