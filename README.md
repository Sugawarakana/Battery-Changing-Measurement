# Introduction
This is a repository for BCL. Currently updating.<br>

# Devices used
Laptop + Maker Pi Pico / Raspberry Pi 3 Model B<br>
2 × AD7746 Capacitance sensors<br>
1 × TCA9548A I2C Multiplexer<br>
Wires<br>
Poron capacitor array<br>

# Connection guides
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
