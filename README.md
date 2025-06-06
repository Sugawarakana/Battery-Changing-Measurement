# Introduction
This is a repository for BCL. Currently updating.<br>

# Devices used
Laptop + Maker Pi Pico / Raspberry Pi 3 Model B<br>
2 × AD7746 Capacitance sensors<br>
1 × TCA9548A I2C Multiplexer<br>
Wires<br>
Poron capacitor array<br>
Metal fixture<br>
Keyence GT2-H12KL High-Accuracy Digital Contact Sensor<br>

# Connection guides
Computer and Pico Pi (or a single Raspberry Pi instead) are used for data processing, logging and monitoring. They are directly connected to an I2C multiplexer (TCA9548A), which promises simultaneous control of 2 capacitance sensors (AD7746). Each of the 2 sensors has 2 channels,  enabling 4-points measurement of the capacitor array.<br>

**Raspberry Pi 3 Model B**<br>
Raspberry Pi-(wire)-TCA9548A-(wire)-AD7746-(wire)-wire-capacitor<br>

**Laptop + Maker Pi Pico**<br>
laptop-(usb-microusb)-Pico-(wire)-TCA9548A-(wire)-AD7746-(wire)-wire-capacitor<br>
Communication via serial monitor<br>

# Settings
hostname: analogK<br>
username: analog<br>
access via mstsc<br>

# Codes:
Monitor.py is for Raspberry Pi<br>
Monitor.ino is for Pico Pi<br>
