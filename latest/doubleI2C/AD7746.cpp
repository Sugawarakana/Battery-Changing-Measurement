// I2Cdev library collection - AD7746 I2C device class
// Based on Analog Devices AD7746 Datasheet, Revision 0, 2005
// 2012-04-01 by Peteris Skorovs <pskorovs@gmail.com>
//
// This I2C device library is using (and submitted as a part of) Jeff Rowberg's I2Cdevlib library,
// which should (hopefully) always be available at https://github.com/jrowberg/i2cdevlib
//
// Changelog:
//     2012-04-01 - initial release

/* ============================================
I2Cdev device library code is placed under the MIT license
Copyright (c) 2012 Peteris Skorovs

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
===============================================
*/

#include "AD7746.h"

/** Default constructor, uses default I2C address.
 * @see AD7746_DEFAULT_ADDRESS
 */
AD7746::AD7746() {
    devAddr = AD7746_DEFAULT_ADDRESS;
    _wire=&Wire;
}

AD7746::AD7746(TwoWire* wire) {
    devAddr = AD7746_DEFAULT_ADDRESS;
    _wire=wire;
}

/** Specific address constructor.
 * @param address I2C address
 * @see AD7746_DEFAULT_ADDRESS
 * @see AD7746_ADDRESS
 */
AD7746::AD7746(uint8_t address) {
    devAddr = address;
    _wire=&Wire;
}

/** Power on and prepare for general usage.
 */
void AD7746::initialize() {
    reset();
}

/** Verify the I2C connection.
 * Make sure the device is connected and responds as expected.
 * @return True if connection is valid, false otherwise
 */
bool AD7746::testConnection() {
    if (readByte(devAddr, AD7746_RA_STATUS, buffer)) {
        return true;
    }
    return false;
}

void AD7746::reset() {   
    
    _wire->beginTransmission(devAddr);
    _wire->write((uint8_t) AD7746_RESET); // send reset

    _wire->endTransmission();
// #endif
    

    
delay(1); //wait a tad for reboot
}


uint32_t AD7746::getCapacitance() {
    uint32_t capacitance;
    
    readBytes(devAddr, 0, 4, buffer);
    capacitance = ((uint32_t)buffer[1] << 16) | ((uint32_t)buffer[2] << 8) | (uint32_t)buffer[3];
    
    return capacitance;
}

int8_t AD7746::readByte(uint8_t devAddr, uint8_t regAddr, uint8_t *data) {
    return readBytes(devAddr, regAddr, 1, data);
}

int8_t AD7746::readBytes(uint8_t devAddr, uint8_t regAddr, uint8_t length, uint8_t *data) {

    uint8_t count = 0;
    uint8_t timeout=0;
    uint32_t t1 = millis();
            // Arduino v1.0.1+, Wire library
            // Adds official support for repeated start condition, yay!

            // I2C/TWI subsystem uses internal buffer that breaks with large data requests
            // so if user requests more than I2CDEVLIB_WIRE_BUFFER_LENGTH bytes, we have to do it in
            // smaller chunks instead of all at once
            for (int k = 0; k < length; k += min((int)length, 10)) {
                _wire->beginTransmission(devAddr);
                _wire->write(regAddr);
                _wire->endTransmission();
                _wire->requestFrom((uint8_t)devAddr, (uint8_t)min((int)length - k, 10));
                for (; _wire->available() && (timeout == 0 || millis() - t1 < timeout); count++) {
                    data[count] = _wire->read();

                }
            }

    return count;
}

bool AD7746::writeByte(uint8_t devAddr, uint8_t regAddr, uint8_t data ) {
    return writeBytes(devAddr, regAddr, 1, &data);
}

bool AD7746::writeBytes(uint8_t devAddr, uint8_t regAddr, uint8_t length, uint8_t* data) {
    uint8_t status=0;
        _wire->beginTransmission(devAddr);
        _wire->write((uint8_t) regAddr); // send address
         for (uint8_t i = 0; i < length; i++) {
            _wire->write((uint8_t) data[i]);
        }
        status = _wire->endTransmission();
        return status == 0;
}


void AD7746::writeCapSetupRegister(uint8_t data) {
    writeByte(devAddr, AD7746_RA_CAP_SETUP, data);
}

void AD7746::writeVtSetupRegister(uint8_t data) {
    writeByte(devAddr, AD7746_RA_VT_SETUP, data);
}


void AD7746::writeExcSetupRegister(uint8_t data) {
    writeByte(devAddr, AD7746_RA_EXC_SETUP, data);
}


void AD7746::writeConfigurationRegister(uint8_t data) {
    writeByte(devAddr, AD7746_RA_CONFIGURATION, data);
}


void AD7746::writeCapDacARegister(uint8_t data) {
    writeByte(devAddr, AD7746_RA_CAP_DAC_A, data);
}

void AD7746::writeCapDacBRegister(uint8_t data) {
    writeByte(devAddr, AD7746_RA_CAP_DAC_B, data);
}

void AD7746::write_register(uint8_t addr,uint8_t data) {
    writeByte(devAddr, addr, data);
}

void AD7746::read_register(uint8_t addr, uint8_t *data) {
    readByte(devAddr, addr, data);
}
