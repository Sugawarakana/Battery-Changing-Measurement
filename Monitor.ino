#include <Wire.h>
#include "AD7746.h"


#define TCA9548A_ADDR 0x70 // Address of multiplexer
#define AD7746_ADDR 0x48 // Address of sensor

unsigned long previousMillis = 0;
// for result output
struct Result{
    float mean;
    float std_dev;
    int count;
    float r0, r1, r2, r3, r4, r5, r6, r7;
};

AD7746 capSensor;

const int numReadings = 16; // Number of samples for mean filtering

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
Result readFilteredCapacitance() {
    Result r;
    float readings[numReadings];
    float fReadings[numReadings];
    uint32_t temp = 0;
    // Take multiple readings for noise reduction
    for (int i = 0; i < numReadings; i++) {
        temp = capSensor.getCapacitance();
        // Convert raw data to capacitance in pF (adjusting for ±4 pF range, starting at 8 pF)
        readings[i] = (float) temp * (4.00 / 0x800000) + 4.00;
        delay(10);// Small delay between readings, the readings will change every delay(10)
    }

    // Sort the readings to get the median

    float mean;

    float sum = 0;
    for(int i = 0; i < numReadings; i++) {
        sum += readings[i];             
    }
    mean = sum / numReadings;

    float variance = 0;
    for (int i = 1; i < numReadings; i++) {       
        variance += pow(readings[i] - mean, 2);
    }
    variance /= numReadings;
    float std_dev = sqrt(variance);
    float lower_bound = mean - 3 * std_dev;
    float upper_bound = mean + 3 * std_dev;
    
    int count = 0;
    for (int i = 0; i < numReadings; i++) {
        if (readings[i] >= lower_bound && readings[i] <= upper_bound) {
            fReadings[count] = readings[i];
            count += 1;
        }
    }
    float fSum = 0;
    for(int i = 0; i < count; i++) {
        fSum += fReadings[i];             
    }
    float fMean = 0;
    fMean = fSum / count;
    r.mean = fMean;
    r.r0 = readings[0];
    r.r1 = readings[1];
    r.r2 = readings[2];
    r.r3 = readings[3];
    r.r4 = readings[4];
    r.r5 = readings[5];
    r.r6 = readings[6];
    r.r7 = readings[7];
    r.std_dev = std_dev;
    r.count = count;
    return r;
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
    unsigned long currentMillis = millis();
    Serial.println(currentMillis);
    tca_select(0);
    capSensor.writeCapSetupRegister(AD7746_CAPEN);
    delay(20);
    Result capacitance0 = readFilteredCapacitance();
    capSensor.writeCapSetupRegister(AD7746_CAPEN | AD7746_CIN2);
    delay(20);
    Result capacitance1 = readFilteredCapacitance();
    capSensor.writeCapSetupRegister(0);
// The problem is when press 0, 0 and 1 both increase, when press 1 nothing happens.
// Delay must be kept in capsetup & read at around 20

    tca_select(1);
    capSensor.writeCapSetupRegister(AD7746_CAPEN);
    delay(20);
    Result capacitance2 = readFilteredCapacitance();
    capSensor.writeCapSetupRegister(AD7746_CAPEN | AD7746_CIN2);
    delay(20);
    Result capacitance3 = readFilteredCapacitance();
    capSensor.writeCapSetupRegister(0);

    // Serial.println(capacitance0.count);
    Serial.print("Capacitance0: ");
    Serial.print(capacitance0.mean, 8);
    Serial.println(" pF");
    // Serial.print(capacitance0.r0, 8);
    // Serial.println(" pF");
    // Serial.print(capacitance0.r1, 8);
    // Serial.println(" pF");
    // Serial.print(capacitance0.r2, 8);
    // Serial.println(" pF");
    // Serial.print(capacitance0.r3, 8);
    // Serial.println(" pF");
    // Serial.print(capacitance0.r4, 8);
    // Serial.println(" pF");
    // Serial.print(capacitance0.r5, 8);
    // Serial.println(" pF");
    // Serial.print(capacitance0.r6, 8);
    // Serial.println(" pF");
    // Serial.print(capacitance0.r7, 8);
    // Serial.println(" pF");


    Serial.print("Capacitance1: ");
    Serial.print(capacitance1.mean, 8);
    Serial.println(" pF");
    
    Serial.print("Capacitance2: ");
    Serial.print(capacitance2.mean, 8);
    Serial.println(" pF");
    Serial.print("Capacitance3: ");
    Serial.print(capacitance3.mean, 8);
    Serial.println(" pF");
    Serial.println(" ");

    Serial.print("std_dev");
    Serial.println(capacitance0.std_dev, 8);
    Serial.println(capacitance1.std_dev, 8);
    Serial.println(capacitance2.std_dev, 8);
    Serial.println(capacitance3.std_dev, 8);
    // Serial.print(capacitance1.r0, 8);
    // Serial.println(" pF");
    // Serial.print(capacitance1.r1, 8);
    // Serial.println(" pF");
    // Serial.print(capacitance1.r2, 8);
    // Serial.println(" pF");
    // Serial.print(capacitance1.r3, 8);
    // Serial.println(" pF");
    // Serial.print(capacitance1.r4, 8);
    // Serial.println(" pF");
    // Serial.print(capacitance1.r5, 8);
    // Serial.println(" pF");
    // Serial.print(capacitance1.r6, 8);
    // Serial.println(" pF");
    // Serial.print(capacitance1.r7, 8);
    // Serial.println(" pF");
    delay(500);
}

