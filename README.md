# Introduction
This is a repository for Battery Control Lab (BCL). Currently updating.  
In the Battery Control Lab, we aim to keep tracking the status of batteries in electric vehicles when charging or discharging. The common idea is that the thickness of battery varies when power changes. However, precise distance sensors are relatively expensive, may not work well for industrial applications. Thus, we're trying to convert length to another physical quantity that can be economically measured.  
Recall the way to calculate capacitance from the high school physical books:
https://github.com/Sugawarakana/BCL/blob/main/Cap.png
We can use the relationship between *d* and *C*. With capacitance measurement, we can estimate current thickness of battery.

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

# Fixture notation
The distance sensor is fixed inside the top layer, and the pin works on top of the bottom layer. Normally the distance between top and bottom is changeless, but in the calibration procedure, we change the tightness of screws to simulate the swell of battery, which compresses the Poron capacitor array.<br>
(From top to bottom)<br>
Top Layer-Solid Metal Stuff-Medium Layer-Battery-Poron Capacitor Array-Bottom Layer

# Calibration Procedures
Install the fixture as shown in the figure, keep all of the screws loose. Reset the Keyence readings to 0.  
Remove the Poron capacitor array and get the original thickness of capacitor pad.  
Put capacitors inside again, tighten the screws lightly with fingers, reset the Keyence readings to 0 and record the capacitance readings on computer screen.  
Tighten the screws evenly by making sure the Keyence readings close to each other, record the distance and capacitance readings on computer screen for several groups.  


# Settings
For Raspberry Pi:
hostname: analogK<br>
username: analog<br>
access via mstsc<br>

# Codes:
Monitor.py is for Raspberry Pi<br>
Data_log.py is for Pico Pi data logging  
Monitor.ino is for Pico Pi<br>
