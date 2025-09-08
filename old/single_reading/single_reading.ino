#include <Wire.h>
#include "AD7746.h"


#define TCA9548A_ADDR 0x70 // Address of multiplexer
#define AD7746_ADDR 0x48 // Address of sensor

AD7746 capSensor;

const int numReadings = 11; // Number of samples for mean filtering

void tca_select(uint8_t bus) {
    if (bus > 7) return;
    Wire.beginTransmission(TCA9548A_ADDR);
    Wire.write(1 << bus);
    Wire.endTransmission();
}

void sensor_init() {
    capSensor.initialize();
 
    // Ensure a proper reset before configuration
    capSensor.reset();
    delay(10);

    // Read factory offset and apply it
    uint32_t offset = capSensor.getCapacitance();
    Serial.print("Factory Offset: ");
    Serial.println(offset);

    // Configure Excitation Voltage to VDD/2 for stability
    capSensor.writeExcSetupRegister(0x0B);
    Serial.println("Excitation Voltage Set to VDD/2");

    // Enable capacitance measurement mode
    capSensor.writeCapSetupRegister(AD7746_CAPEN);
    Serial.println("Capacitance Measurement Enabled");

    // Set Continuous Conversion Mode
    capSensor.writeConfigurationRegister(AD7746_MD_CONTINUOUS_CONVERSION);
    Serial.println("Continuous Conversion Mode Enabled");

    // Configure Cap DAC to shift the reference to 8 pF (so we measure 4 - 12 pF)
    uint8_t dacValue = 60;  // (8 pF / 0.133858 pF per unit) ≈ 60
    capSensor.writeCapDacARegister(dacValue | AD7746_DACAEN); // Enable Cap DAC A
    Serial.println("Cap DAC A Configured to 8 pF Offset");

    Serial.println("AD7746 Configuration Complete. Reading Capacitance...");

}

// **Function to get filtered capacitance value using median filtering**
float readFilteredCapacitance() {
    uint32_t readings[numReadings];

    // Take multiple readings for noise reduction
    for (int i = 0; i < numReadings; i++) {
        readings[i] = capSensor.getCapacitance();
        delay(10); // Small delay between readings
    }

    // Sort the readings to get the median
    for (int i = 0; i < numReadings - 1; i++) {
        for (int j = i + 1; j < numReadings; j++) {
            if (readings[i] > readings[j]) {
                uint32_t temp = readings[i];
                readings[i] = readings[j];
                readings[j] = temp;
            }
        }
    }

    uint32_t medianReading = readings[numReadings / 2]; // Get median value
    uint32_t meanReading;
    int hi = numReadings - 1;
    int lo = 0;
    int sum = 0;
    for(int i = lo + 2; i < hi - 1; i++) {
        sum += readings[i];             
    }
    meanReading = sum / (hi - lo - 3);
    // Convert raw data to capacitance in pF (adjusting for ±4 pF range)
    float capValue = ((float)meanReading - 0x800000) * (4.0 / 0x800000);
    return capValue + 8.0; // Adding 8pF center offset
}

void print_all() {   
    Serial.print("Capacitance0: ");
    Serial.print(capacitance0);
    Serial.println(" pF");
    Serial.print("Capacitance1: ");
    Serial.print(capacitance1);
    Serial.println(" pF");
    Serial.print("Capacitance2: ");
    Serial.print(capacitance2);
    Serial.println(" pF");
    Serial.print("Capacitance3: ");
    Serial.print(capacitance3);
    Serial.println(" pF");
    Serial.println(" ")
}


void setup() {
    Serial.begin(115200);
    Wire.begin();
    tca_select(0);
    sensor_init();
    delay(10);
    tca_select(1);
    sensor_init();
    capSensor.writeCapSetupRegister(0);
}


void loop() {
    tca_select(0);
    capSensor.writeCapSetupRegister(AD7746_CAPEN);
    float capacitance0 = readFilteredCapacitance();
    capSensor.writeCapSetupRegister(AD7746_CAPEN | AD7746_CIN2);
    float capacitance1 = readFilteredCapacitance();
    capSensor.writeCapSetupRegister(0);
    
    tca_select(1);
    capSensor.writeCapSetupRegister(AD7746_CAPEN);
    float capacitance2 = readFilteredCapacitance();
    capSensor.writeCapSetupRegister(AD7746_CAPEN | AD7746_CIN2);
    float capacitance3 = readFilteredCapacitance();
    capSensor.writeCapSetupRegister(0);

    print_all();

    delay(500);
}

