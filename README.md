Arduino Temperature Monitor - SD Logging Branch
---

![Pic](http://i.imgur.com/55Budpy.jpg)

Example log data written to 'logN.txt' where N is increasing with button presses.

```
540 -4.1 22.3
1567 -4.1 22.3
2575 -4.0 22.3
3582 -4.1 22.3
4590 -4.0 22.3
5598 -4.1 22.3
6605 -4.0 22.3
7614 -4.1 22.3
8622 -4.0 22.3
9629 -4.1 22.3
10637 -4.0 22.3
11644 -4.1 22.3
12652 -4.0 22.3
13660 -4.0 22.3
14667 -4.0 22.3
15675 -4.0 22.3
16683 -4.0 22.3
17690 -4.0 22.3
18699 -4.0 22.3
19706 -4.0 22.3
...
```

* In this branch, I removed the LCD / LED stuff, since I'm  working with the SD Card and just want to log temperature data for the rest of winter '13.

In terms of space on SD Card, a worst case entry is `4294967295 -273.1 -273.1`

25 chars to write that, 1 byte a char = 25 bytes per line ~ 25 bytes/sec
`50 days * (25 bytes/sec) = 102.996826 megabytes`


A single entry will reasonably take at most 25 chars. At a rate of 1 Hz, it comes out to about 2 Mb a day, so in 25 days when I get back it should be around 50 Mb. I've left it running with a 4 Gb SD card so it should be fine.

Code is set up such that if power resets on it, it'll go to next new log file and start writing there.


Schematic
---

![Basic schematic](http://i.imgur.com/nZZ8zjA.png)

Parts
---

* Arduino Uno
* Arduino Wireless SD Shield
* IIC/I2C/TWI 1602 Serial LCD Module Display by zitrades
* Red LED, Green LED
* 2 470 ohm resistors for LEDs
* 2 10K ohm resistors for thermistor voltage divider circuits
* 330K ohm resistor for grounded pushbutton circuit
* 2 Amico 10K Ohm 3% Temperature Measurement NTC Thermistors
* 3 pin header male/female connectors for thermistors and future photo-resistor.
* Pushbutton switch to turn LCD backlight on and off
* Assortment of solid-core ~24 gauge wiring.

Installation
---
Add folders in `libraries/` to your arduino libraries folder.

