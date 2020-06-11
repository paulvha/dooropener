# Door opener

## ===========================================================

A program to create a door-opener, based on UNO, to open a door with
a password and/or NFC card. The solution is using a keypad, MFRC522
and LCD (16 x 2). These are often used hardware components that can be
sourced from many places. The dooropener can be connected through a
relay or any other electronic circuit. THis is outside the scope of the
project.
<br> A detailed description of the options and findings are in dooropener.odt

## Getting Started
There was someone on a forum looking for help to get a door-opener, based serial,
keypad and MRFC522 card. He had some code, which was not working, copy / pasted
together.  The project looked interesting me, also I had all the components,
except a door opener, and had couple of hours on my hand. I have extended
the code with more functions and this document is the write up of the small project.

## Prerequisites
Depending the exact sourced hardware components
Keypad
 https://github.com/Chris--A/Keypad
 info : http://playground.arduino.cc/Code/Keypad

LCD
 info : https://www.makerguides.com/character-i2c-lcd-arduino-tutorial/
 info: https://www.arduinolibraries.info/libraries/liquid-crystal-i2-c
 https://github.com/marcoschwartz/LiquidCrystal_I2C

MFRC522
 info: https://playground.arduino.cc/Learning/MFRC522/
 https://github.com/miguelbalboa/rfid

## Software installation
Obtain the zip and install like any other

## Program usage
### Program options
Please see the description in the top of the sketch and read the documentation (odt)

## Versioning

### version 1.0 / June 202
 * Initial version Arduino

## CO- Author
 * Paul van Haastrecht (paulvha@hotmail.com)

## License
This project is licensed under the GNU GENERAL PUBLIC LICENSE 3.0

