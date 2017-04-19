Laser Monitor
===================

![Laser Monitor Controller](/images/IMG_6241.jpg?raw=true "Laser Monitor Controller")
![Laser Monitor Controller](/images/IMG_6240.jpg?raw=true "Laser Monitor Controller")

This device has three sensors connected via RJ45 connectors:

Port 1: Laser PSU
-----------------

|RJ45 PIN  | FUNCTION | WIRE COLOUR |
|--------  | -------- | ----------- |
|1         |          |             |
|2         | GND      | Black       |
|3         |          |             |
|4         | +5v      | Red         |
|5         |          |             |
|6         | PWM      | Green       |
|7         |          |             |
|8         | TTL      | Yellow      |

Wire colours above are for the cable that connects onto the Laser PSU.
PWM indicates the laser PSU's current power level setting. It remains active even when the laser is not firing.
TTL is the "enable" signal, is active low, and indicates that the laser is firing.

Port 2 - Filter box
-------------------

Connects to two BMP180 digital pressure/temperature sensors located at the inlet and outlet of the filter box. Comparing these pressures gives an indication of how clogged the filters are.

|RJ45 PIN  | FUNCTION        |
|--------  | --------        |
|1         | GND             |
|2         | +3.3v           |
|3         | GND             |
|4         | +3.3v           |
|5         | SDA (sensor 1)  |
|6         | SCL (sensor 1)  |
|7         | SDA (sensor 2)  |
|8         | SCL (sensor 2)  |



Port 3 - Chiller bath
---------------------

This is a simple temperature probe indicating the water temperature in the reservoir.

|RJ45 PIN  | FUNCTION  |
|--------  | --------  |
|1         |           |
|2         | GND       |
|3         |           |
|4         | +3.3v     |
|5         |           |
|6         | OneWire   |
|7         |           |
|8         |           |


ESP8266 Pin Allocation
----------------------

| PIN | Logical| Description   |
|-----|--------|---------------|
| D0  | GPIO16 | PWM IN        |
| D1  | GPIO5  | TTL           |
| D2  | GPIO4  | SDA sensor 1  |
| D3  | GPIO0  |               |
| D4  | GPIO2  | SCL sensor 1  |
| D5  | GPIO14 | onewire       |
| D6  | GPIO12 | SDA - sensor 2|
| D7  | GPIO13 | SCL - sensor 2|
| D8  | GPIO15 |               |
| D9  | GPIO3  |               |
| D10 | GPIO1  |               |


