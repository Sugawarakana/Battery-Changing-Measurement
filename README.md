# Introduction
This is a repository for BCL Group. Currently updating.

# Devices used
Laptop + Maker Pi Pico / Raspberry Pi 3 Model B
2 × AD7746 Capacitance sensors
1 × TCA9548A I2C Multiplexer
Wires
Poron capacitor array

# Connection guides
**Raspberry Pi 3 Model B**<br>
Raspberry Pi-(wire)-TCA9548A-(wire)-AD7746-(wire)-wire-capacitor

**Laptop + Maker Pi Pico**
laptop-(usb-microusb)-Pico-(wire)-TCA9548A-(wire)-AD7746-(wire)-wire-capacitor
Communication via serial monitor

# Settings
hostname: analogK
username: analog
access via mstsc

# Codes:
Monitor.py is for Raspberry Pi
Monitor.ino is for Pico Pi
