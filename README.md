# Introduction

This is the official repository for the Battery Control Lab (BCL), currently undergoing updates.

The latest codes are available in the folder 'latest' (last updated September 8, 2024).

You can input a number via serial to adjust the capacitance offset. Each unit changes the offset by 0.133858 pF (default: 30 units, providing an adjustable range of 4-8 pF).

In the Battery Control Lab, we aim to keep tracking the status of batteries in electric vehicles when charging or discharging. The common idea is that the thickness of battery varies when power changes. However, precise distance sensors are relatively expensive, may not work well for industrial applications. Thus, we're trying to convert length to another physical quantity that can be economically measured.  

Recall the way to calculate capacitance from the high school physical books:  

![Capacitance](./images/Cap.png)  

We can use the relationship between *d* and *C*. With capacitance measurement, we can estimate current thickness of battery.  

The ongoing work is divided into several steps:  

1. Let AD7746 read a single capacitance;
2. Let AD7746 read two capacitances alternatively through different channels;
3. Let microprocessor connect two AD7746 chips at the same address, enabling 4-points reading;
4. Use 4-points reading pad to calibrate the relationship between *d* and *C*;
5. Optimize the delay and error;
6. Monitering the charging and discharging data.

# Devices used

Laptop + Maker Pi Pico / Raspberry Pi 3 Model B (old)  

2 × AD7746 Capacitance sensors  

1 × TCA9548A I2C Multiplexer (old)  

Wires (old)  

Shielded wires

Copper foil shield

Poron capacitor array  

3-Layer Metal fixture  

Keyence GT2-H12KL High-Accuracy Digital Contact Sensor  

# Connection guides

Computer and Pico Pi (or a single Raspberry Pi instead) are used for data processing, logging and monitoring. They are directly connected to an I2C multiplexer (TCA9548A), which promises simultaneous control of 2 capacitance sensors (AD7746). Each of the 2 sensors has 2 channels,  enabling 4-points measurement of the capacitor array.  

**Raspberry Pi 3 Model B**  

Raspberry Pi-(wire)-TCA9548A-(wire)-AD7746-(wire)-wire-capacitor (old)

**Laptop + Maker Pi Pico**  

Computer-(usb-microusb)-Pico-(wire)-TCA9548A-(wire)-AD7746-(wire)-wire-capacitor (old)

**Computer-(usb-microusb)-Pico-(double i2c wire)-AD7746-(wire)-wire-capacitor (latest)**

Communication via serial monitor  

# AD7746 guides

The AD7746 has a fixed default I2C address of 0x48. Therefore, using multiple sensors requires either multiple I2C channels or an I2C address multiplexer. However, implementing the latter led to significant fluctuations in readings, prompting its abandonment.  

The datasheet is in the **guides** folder.  

Single conversion means reading a single data point from the registers, then clearing the configuration register. If you want it to run continuously, continuous conversion might be an option. However, weird, unstable data might be obtained when different sensors work together. Therefore, rewriting the configuration register after every conversion might be the most reliable approach.

# TCA9548A guides

Used to split the data and clock line (SDA & SCL). The datasheet is in the **guides** folder. Set the switch by send a 8-bit data to the designated address. Default address 0x70.

# Fixture notation

The distance sensor is fixed inside the top layer, and the pin works on top of the bottom layer. Normally the distance between top and bottom is changeless, but in the calibration procedure, we change the tightness of screws to simulate the swell of battery, which compresses the Poron capacitor array.  

(From top to bottom)  

Top Layer-Solid Metal Stuff-Medium Layer-Battery-Poron Capacitor Array-Bottom Layer  

# Calibration procedures

Install the fixture as shown in the figure, keep all of the screws loose. Reset the Keyence readings to 0.  

Remove the Poron capacitor array and get the original thickness of capacitor pad.  

Put capacitors inside again, tighten the screws lightly with fingers, reset the Keyence readings to 0 and record the capacitance readings on computer screen.  

Tighten the screws evenly by making sure the Keyence readings close to each other, record the distance and capacitance readings on computer screen for several groups.  

# Sampling delay and standard error

Sampling delay is set to 62.0 ms, with a minimized RMS Noise of 1.5 aF/sqrt(Hz) with respect to the datasheet. In a real experimental environment, the variance was 1.20E-07 pF^2.

# Data Logging

The Pico Pi outputs serial data via a micro USB cable. The Python script capacitance_log.py utilizes a serial protocol for data transmission. The default COM port is COM4, with a baud rate of 115200. The measured capacitances and their corresponding timestamps are then stored in capacitance_data.csv

# Remote settings

For Raspberry Pi:  

hostname: analogK  

username: analog  

access via mstsc  

# About this folder

Monitor.py is for Raspberry Pi (old)  

Data_log.py is for Pico Pi data logging (old)  

Monitor.ino is for Pico Pi (old)  

**doubleI2C.ino is for Pico Pi 4 sensor stable reading (latest)**
