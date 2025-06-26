#include <Wire.h>
#include "AD7746.h"


#define TCA9548A_ADDR 0x70 // Address of multiplexer
#define AD7746_ADDR 0x48 // Address of sensor

const int Pin3_AD0 = 3;
const int Pin2_AD1 = 2;
volatile bool newDataAvailable_0 = false;
volatile bool newDataAvailable_1 = false;
volatile bool select_0 = false;
volatile bool select_1 = false;
int reading_count = 0; // To record the status of array
const int numReadings = 16; // Number of samples for mean filtering
float readings[4][numReadings];
float fReadings[4][numReadings];



void dataReadyISR_0() {
    newDataAvailable_0 = true;

}
void dataReadyISR_1() {
    newDataAvailable_1 = true;
}


AD7746 capSensor;


void tca_select(uint8_t bus) {
    Wire.beginTransmission(TCA9548A_ADDR);
    Wire.write(1 << bus);
    Wire.endTransmission();
}

void sensor_init() {
    capSensor.initialize();
 
    // Ensure a proper reset before configuration
    capSensor.reset();
    delay(20);

    // Read factory offset and apply it
    uint32_t offset = capSensor.getCapacitance();
    Serial.print("Factory Offset: ");
    Serial.println(offset);

    // Configure Excitation Voltage to VDD/2 for stability
    capSensor.writeExcSetupRegister(0x2B);
    Serial.println("Excitation Voltage Set to VDD/2");//vdd/4

    // Enable capacitance measurement mode
    capSensor.writeCapSetupRegister(AD7746_CAPEN);
    Serial.println("Capacitance Measurement Enabled");

    // Set Continuous Conversion Mode, conversion time to 109.6ms
    capSensor.writeConfigurationRegister(AD7746_CAPF_62P0 | AD7746_MD_CONTINUOUS_CONVERSION);
    Serial.println("Continuous Conversion Mode Enabled");

    // Configure Cap DAC to shift the reference to 8 pF (so we measure 4 - 12 pF)
    uint8_t dacValue = 120;  // (8 pF / 0.133858 pF per unit) ≈ 60 12pF
    capSensor.writeCapDacARegister(dacValue | AD7746_DACAEN); // Enable Cap DAC A
    Serial.println("Cap DAC A Configured to 8 pF Offset");

    Serial.println("AD7746 Configuration Complete. Reading Capacitance...");

}




void readSingleCapacitance() {
    uint32_t temp = capSensor.getCapacitance();
    int cap_num = reading_count % 6;
    int read_num = reading_count / 6;
    // cap_num: 1-->cap0 
    //          2-->cap1 
    //          4-->cap2
    //          5-->cap3
    // for 0, 3, the reading after selection experiences large variance in voltage, result in large noise, drop the two sets of data
    if (cap_num == 1 || cap_num == 2){
        readings[cap_num - 1][read_num] = (float) temp * (4.00 / 0x800000) + 8.00;
    }
    if (cap_num == 4 || cap_num == 5){
        readings[cap_num - 2][read_num] = (float) temp * (4.00 / 0x800000) + 8.00;
    }
    switch (cap_num){
        case 0:
            newDataAvailable_0 = false;
            break;
        case 1:
            capSensor.writeCapSetupRegister(AD7746_CAPEN | AD7746_CIN2);
            newDataAvailable_0 = false;
            break;
        case 2:
            capSensor.writeCapSetupRegister(0);
            tca_select(1);
            // delay(100);
            capSensor.writeCapSetupRegister(AD7746_CAPEN);

            newDataAvailable_0 = false;
            
            break;
        case 3:
            newDataAvailable_1 = false;
            break;
        case 4:
            capSensor.writeCapSetupRegister(AD7746_CAPEN | AD7746_CIN2);
            newDataAvailable_1 = false;
            break;
        case 5:
            capSensor.writeCapSetupRegister(0);
            tca_select(0);
            capSensor.writeCapSetupRegister(AD7746_CAPEN);
            newDataAvailable_1 = false;
            break;
    }

}

// **Function to get filtered capacitance value using median filtering**
void readFilteredCapacitance() {

    float mean[4] = {};
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
    float std_dev[4] = {};
    float lower_bound[4] = {};
    float upper_bound[4] = {};
    int count[4] = {};
    // for (int i = 0; i < numReadings; i++){
    //     Serial.println(readings[0][i]);
    // }
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
    // Serial.println("count0: ");
    // for (int k = 0; k < 8; k++) {
    //     Serial.println(readings[1][k], 8);
    // }
    // for (int k = 0; k < 8; k++) {
    //     Serial.println(readings[2][k], 8);
    // }
    Serial.print("Variance for Cap0: ");
    Serial.println(variance[0], 5);
    Serial.print("Variance for Cap1: ");
    Serial.println(variance[1], 5);
    Serial.print("Variance for Cap2: ");
    Serial.println(variance[2], 5);
    Serial.print("Variance for Cap3: ");
    Serial.println(variance[3], 5);

    Serial.print("Capacitance0: ");
    Serial.println(fMean[0], 5);
    Serial.print("Capacitance1: ");
    Serial.println(fMean[1], 5);
    Serial.print("Capacitance2: ");
    Serial.println(fMean[2], 5);
    Serial.print("Capacitance3: ");
    Serial.println(fMean[3], 5);
    Serial.println(" ");
    
}


void setup() {
    Serial.begin(115200);
    Wire.begin();
    tca_select(0);
    sensor_init();
    delay(20);
    tca_select(1);
    sensor_init();
    capSensor.writeCapSetupRegister(0);
    pinMode(Pin3_AD0, INPUT_PULLUP);
    pinMode(Pin2_AD1, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(Pin3_AD0), dataReadyISR_0, FALLING);
    attachInterrupt(digitalPinToInterrupt(Pin2_AD1), dataReadyISR_1, FALLING);
    tca_select(0);
    capSensor.writeCapSetupRegister(AD7746_CAPEN);
}




void loop() {
    noInterrupts(); // 
    bool isDataReady_0 = newDataAvailable_0;
    bool isDataReady_1 = newDataAvailable_1;
    interrupts(); //  restart interrupts
 
    if (reading_count == 6 * numReadings){
        // Enough readings for data processing
        readFilteredCapacitance();
        reading_count = 0;
    }
    if (isDataReady_1 || isDataReady_0){
        // Serial.print(isDataReady_0);
        // Serial.println(reading_count);
        readSingleCapacitance();
        reading_count += 1;
    }

}
