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

# TODO
- Add more documentation
