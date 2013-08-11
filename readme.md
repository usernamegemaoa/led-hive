# LED Hive

This is the software part of the "LED Hive" project.
LED Hive is an installation of RGB LEDs in plastic drinking cups. Soldered mostly on OHM2013.

It consists of:

- 64 RGB LEDs
- Raspberry PI as a Controller
- Lots of soldering

Some videos of it on youtube:

- <http://www.youtube.com/watch?v=prjd5dCNWSg>
- <http://www.youtube.com/watch?v=8Kv1bou8324>

Some pictures:

- <http://frupic.frubar.net/30036>
- <http://frupic.frubar.net/30034>
- <http://frupic.frubar.net/30033>


## Circuit

- <http://frupic.frubar.net/shots/30040.jpg>

## LHC

LED Hive Control is a small C daemon that refreshes the state to the LEDs using the GPIOs of the Raspberry PI. It can receive messages via UDP to update the state.


The following GPIOs are used as:

- 8-Bit Bus: 27, 18, 17, 15, 14, 4, 3, 2
- Strobe Red: 23
- Strobe Green: 24
- Strobe Blue: 22
- Strobe Row: 10

The UDP protocol is very simpel. It consists of a header-magic "BPSW" and 64 x 3 Bytes for each color (RGB) for each LED. So the total length is 196 Bytes.
The value for each LED Color can be 0x00 for off and 0x01 for on.

	
 	+---+---+---+---+---------+---------+---------+---------+-//-+---------+ 
 	| B   P   S   W | LED00_R | LED00_G | LED00_B | LED01_R |  … | LED63_B |
 	+---+---+---+---+---------+---------+---------+---------+-//-+---------+


## py-only-with-sysfs

The first attempt was to use the GPIOs over sysfs from python.
This was to slow, the code remains for demonstration purposes.
