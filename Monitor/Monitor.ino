#include <Wire.h>
#include "AD7746.h"


#define TCA9548A_ADDR 0x70 // Address of multiplexer
#define AD7746_ADDR 0x48 // Address of sensor

unsigned long previousMillis = 0;
// for result output
struct Result{
    float cap0, cap1, cap2, cap3;
    // float mean;
    // float std_dev;
    // int count;
    // float r0, r1, r2, r3, r4, r5, r6, r7;
};

AD7746 capSensor;

const int numReadings = 8; // Number of samples for mean filtering

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


void wait(){
    while (capSensor.readStatusRegister() & 0x1) {
        delayMicroseconds(100);
    }
}

// **Function to get filtered capacitance value using median filtering**
Result readFilteredCapacitance() {
    Result r;
    float readings[4][numReadings];
    float fReadings[4][numReadings];
    uint32_t temp = 0;
    // Take multiple readings for noise reduction
    for (int i = 0; i < numReadings; i++) {
        tca_select(0);
        capSensor.writeCapSetupRegister(AD7746_CAPEN);
        wait();
        temp = capSensor.getCapacitance();
        readings[0][i] = (float) temp * (4.00 / 0x800000) + 4.00;

        capSensor.writeCapSetupRegister(AD7746_CAPEN | AD7746_CIN2);
        wait();
        temp = capSensor.getCapacitance();
        readings[1][i] = (float) temp * (4.00 / 0x800000) + 4.00;
        capSensor.writeCapSetupRegister(0);
        tca_select(1);
        capSensor.writeCapSetupRegister(AD7746_CAPEN);
        wait();
        temp = capSensor.getCapacitance();
        readings[2][i] = (float) temp * (4.00 / 0x800000) + 4.00;
        capSensor.writeCapSetupRegister(AD7746_CAPEN | AD7746_CIN2);
        wait();
        temp = capSensor.getCapacitance();
        readings[3][i] = (float) temp * (4.00 / 0x800000) + 4.00;
        capSensor.writeCapSetupRegister(0);
    }

    // Sort the readings to get the median

    float mean[4];
    float sum[4] = {};
    for (int i = 0; i < 4; i++){
        for(int j = 0; j < numReadings; j++) {
            sum[i] += readings[i][j];             
        }
        mean[i] = sum[i] / numReadings;
    }

    float variance[4] = {};
    for (int i = 0; i < 4; i++) { 
        for (int j = 0; j < numReadings; j++) {       
            variance[i] += pow(readings[i][j] - mean[i], 2);
        }
        variance[i] /= numReadings;
    }
    float std_dev[4];
    float lower_bound[4];
    float upper_bound[4];
    int count[4] = {};
    for (int i = 0; i < 4; i++){
        std_dev[i] = sqrt(variance[i]);
        lower_bound[i] = mean[i] - 3 * std_dev[i];
        upper_bound[i] = mean[i] + 3 * std_dev[i];
        for (int j = 0; j < numReadings; j++) {
            if (readings[i][j] >= lower_bound[i] && readings[i][j] <= upper_bound[i]) {
                fReadings[i][count[i]] = readings[i][j];
                count[i] += 1;
            }
        }
    }
    float fSum[4] = {};
    float fMean[4] = {};
    for (int i = 0; i < 4; i++){
        for(int j = 0; j < count[i]; j++) {
                fSum[i] += fReadings[i][j];             
        }
        fMean[i] = fSum[i] / count[i];
    }
    r.cap0 = fMean[0];
    r.cap1 = fMean[1];
    r.cap2 = fMean[2];
    r.cap3 = fMean[3];
    
    

    // fMean = fSum / count;
    // r.mean = fMean;
    // r.r0 = readings[0];
    // r.r1 = readings[1];
    // r.r2 = readings[2];
    // r.r3 = readings[3];
    // r.r4 = readings[4];
    // r.r5 = readings[5];
    // r.r6 = readings[6];
    // r.r7 = readings[7];
    // r.std_dev = std_dev;
    // r.count = count;
    return r;
}


void setup() {
    Serial.begin(115200);
    Wire.begin();
    tca_select(0);
    sensor_init();
    delay(11);
    tca_select(1);
    sensor_init();
    capSensor.writeCapSetupRegister(0);
}




void loop() {
    // tca_select(0);
    // capSensor.writeCapSetupRegister(AD7746_CAPEN);
    // wait();
    // Result capacitance0 = readFilteredCapacitance();
    // capSensor.writeCapSetupRegister(AD7746_CAPEN | AD7746_CIN2);
    // wait();
    // Result capacitance1 = readFilteredCapacitance();
    // capSensor.writeCapSetupRegister(0);

    // tca_select(1);
    // capSensor.writeCapSetupRegister(AD7746_CAPEN);
    // wait();
    // Result capacitance2 = readFilteredCapacitance();
    // capSensor.writeCapSetupRegister(AD7746_CAPEN | AD7746_CIN2);
    // wait();
    // Result capacitance3 = readFilteredCapacitance();
    // capSensor.writeCapSetupRegister(0);

    // // Serial.println(capacitance0.count);
    // Serial.print("Capacitance0: ");
    // Serial.print(capacitance0.mean, 8);
    // Serial.println(" pF");
    // // Serial.print(capacitance0.r0, 8);
    // // Serial.println(" pF");
    // // Serial.print(capacitance0.r1, 8);
    // // Serial.println(" pF");
    // // Serial.print(capacitance0.r2, 8);
    // // Serial.println(" pF");
    // // Serial.print(capacitance0.r3, 8);
    // // Serial.println(" pF");
    // // Serial.print(capacitance0.r4, 8);
    // // Serial.println(" pF");
    // // Serial.print(capacitance0.r5, 8);
    // // Serial.println(" pF");
    // // Serial.print(capacitance0.r6, 8);
    // // Serial.println(" pF");
    // // Serial.print(capacitance0.r7, 8);
    // // Serial.println(" pF");


    // Serial.print("Capacitance1: ");
    // Serial.print(capacitance1.mean, 8);
    // Serial.println(" pF");
    
    // Serial.print("Capacitance2: ");
    // Serial.print(capacitance2.mean, 8);
    // Serial.println(" pF");
    // Serial.print("Capacitance3: ");
    // Serial.print(capacitance3.mean, 8);
    // Serial.println(" pF");
    // Serial.println(" ");

    // // Serial.print("std_dev");
    // // Serial.println(capacitance0.std_dev, 8);
    // // Serial.println(capacitance1.std_dev, 8);
    // // Serial.println(capacitance2.std_dev, 8);
    // // Serial.println(capacitance3.std_dev, 8);
    // // Serial.print(capacitance1.r0, 8);
    // // Serial.println(" pF");
    // // Serial.print(capacitance1.r1, 8);
    // // Serial.println(" pF");
    // // Serial.print(capacitance1.r2, 8);
    // // Serial.println(" pF");
    // // Serial.print(capacitance1.r3, 8);
    // // Serial.println(" pF");
    // // Serial.print(capacitance1.r4, 8);
    // // Serial.println(" pF");
    // // Serial.print(capacitance1.r5, 8);
    // // Serial.println(" pF");
    // // Serial.print(capacitance1.r6, 8);
    // // Serial.println(" pF");
    // // Serial.print(capacitance1.r7, 8);
    // // Serial.println(" pF");
    Result capacitances = readFilteredCapacitance();
    Serial.print("Capacitance0: ");
    Serial.print(capacitances.cap0, 8);
    Serial.println(" pF");
    Serial.print("Capacitance1: ");
    Serial.print(capacitances.cap1, 8);
    Serial.println(" pF");
    Serial.print("Capacitance2: ");
    Serial.print(capacitances.cap2, 8);
    Serial.println(" pF");
    Serial.print("Capacitance3: ");
    Serial.print(capacitances.cap3, 8);
    Serial.println(" pF");
    Serial.println(" ");
    delay(500);
}

