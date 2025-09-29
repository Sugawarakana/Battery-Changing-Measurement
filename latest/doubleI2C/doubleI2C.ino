#include <Wire.h>
#include "AD7746.h"
#include "RPi_Pico_TimerInterrupt.h"
//#include <FlexWire.h>
#define TIMER_INTERVAL_MS 50L // must be more than sampling rate by 2
// #define TCA9548A_ADDR 0x70 // Address of multiplexer
#define AD7746_ADDR 0x48 // Address of sensor

// Define I2C1 pins 
#define SDA1 6
#define SCL1 7

RPI_PICO_Timer ITimer(0);
uint32_t total = 0;
const int Pin3_AD0 = 15;
const int Pin2_AD1 = 16;
volatile bool TimeToStartConversion = false;
volatile bool newDataAvailable_0 = false;
volatile bool newDataAvailable_1 = false;
volatile bool select_B_0 = false;
// volatile bool select_B_1 = false;
int reading_count = 0; // To record the status of array
const int numReadings = 100; // Number of samples for mean filtering
const int numCaps = 4;
volatile uint8_t dacValue = 30;
volatile float readings[numCaps][numReadings];
// float fReadings[numCaps][numReadings];
//TwoWire Wire1(6, 7);



//
bool TimerHandler(struct repeating_timer *t) 
{
    TimeToStartConversion=true;
    return true;
}

void dataReadyISR_0(){
    newDataAvailable_0 = true;
}
void dataReadyISR_1() {
     newDataAvailable_1 = true;
}


AD7746 capSensor0(&Wire);  // I2C0
AD7746 capSensor1(&Wire1); // I2C1

void sensor0_init() {
    capSensor0.initialize();
    capSensor0.reset();
    delay(20);

//     // Read factory offset and apply it
    uint32_t offset = capSensor0.getCapacitance();
    Serial.print("Factory Offset: ");
    Serial.println(offset);

//     // Configure Excitation Voltage to VDD/2 for stability
    capSensor0.writeExcSetupRegister(0x0B);
    // capSensor0.writeExcSetupRegister(0x03);
//     Serial.println("Excitation Voltage Set to VDD/2");//vdd/4


//     Serial.println("Capacitance Measurement Enabled");

//     // Set Continuous Conversion Mode, conversion time to 109.6ms
    capSensor0.writeConfigurationRegister(AD7746_CAPF_62P0 | AD7746_MD_SINGLE_CONVERSION);
//     Serial.println("Continuous Conversion Mode Enabled");

//     // Configure Cap DAC to shift the reference to 8 pF (so we measure 4 - 12 pF)
    // uint8_t dacValue = 60;  // (8 pF / 0.133858 pF per unit) ≈ 60 12pF
    capSensor0.writeCapDacARegister(dacValue | AD7746_DACAEN); // Enable Cap DAC A

    capSensor0.writeCapSetupRegister(AD7746_CAPEN);
    // capSensor0.writeCapSetupRegister(0);
    
    // uint32_t s = capSensor0.readStatusRegister();
    // Serial.println(s);
}

void sensor1_init() {
    capSensor1.initialize();
    // Ensure a proper reset before configuration
    capSensor1.reset();
    delay(20);

    // Read factory offset and apply it
    uint32_t offset = capSensor1.getCapacitance();
    Serial.print("Factory Offset: ");
    Serial.println(offset);

    // Configure Excitation Voltage to VDD/2 for stability
    capSensor1.writeExcSetupRegister(0x0B);
  
    // Serial.println("Excitation Voltage Set to VDD/2");//vdd/4

    // Enable capacitance measurement mode
    // capSensor1.writeCapSetupRegister(AD7746_CAPEN);
    // Serial.println("Capacitance Measurement Enabled");

    // Set Continuous Conversion Mode, conversion time to 109.6ms
    capSensor1.writeConfigurationRegister(AD7746_CAPF_62P0 | AD7746_MD_SINGLE_CONVERSION);
    // Serial.println("Continuous Conversion Mode Enabled");

    // Configure Cap DAC to shift the reference to 8 pF (so we measure 4 - 12 pF)
    // uint8_t dacValue = 60;  // (8 pF / 0.133858 pF per unit) ≈ 60 12pF
    capSensor1.writeCapDacARegister(dacValue | AD7746_DACAEN); // Enable Cap DAC A
    // Serial.println("Cap DAC A Configured to 8 pF Offset");

    // Serial.println("AD7746 Configuration Complete. Reading Capacitance...");
    capSensor1.writeCapSetupRegister(0);

}





// **Function to get filtered capacitance value using median filtering**
void readFilteredCapacitance() {

    float mean[numCaps] = {};
    float sum[numCaps] = {};

    for (int i = 0; i < numCaps; i++){
        sum[i]=0; // NEED TO INITIALIZE THIS TO ZERO SIEGELJB
        for(int j = 0; j < numReadings; j++) {
            sum[i] += readings[i][j];             
        }
        mean[i] = sum[i] / (float) numReadings;
    }

    float variance[numCaps] = {};
    for (int i = 0; i < numCaps; i++) { 
        variance[i]=0; // NEED TO INITIALIZE THIS TO ZERO SIEGELJB
        for (int j = 0; j < numReadings; j++) {       
            variance[i] += pow(readings[i][j] - mean[i], 2);
        }
        variance[i] /= (float)numReadings;
    }

    // Serial.print("Variance for Cap0: ");
    // Serial.println(variance[0], 5);
    // Serial.print("Variance for Cap1: ");
    // Serial.println(variance[1], 5);

    float std_dev[numCaps] = {};
    float lower_bound[numCaps] = {};
    float upper_bound[numCaps] = {};
    int count[numCaps] = {};
    // for (int i = 0; i < numReadings; i++){
    //     Serial.println(readings[0][i]);
    // }

    float fSum[numCaps] = {};
    float fMean[numCaps] = {};
    for (int i = 0; i < numCaps; i++){
        std_dev[i] = sqrt(variance[i]);
        fSum[i]=0;
        fMean[i] =0;
        count[i]=0;
        lower_bound[i] = mean[i] - 3 * std_dev[i];
        upper_bound[i] = mean[i] + 3 * std_dev[i];
        for (int j = 0; j < numReadings; j++) {
            if (readings[i][j] >= lower_bound[i] && readings[i][j] <= upper_bound[i]) {
                // fReadings[i][count[i]] = readings[i][j];
                count[i] += 1;
                fSum[i]+=readings[i][j];
            }
            fMean[i]=fSum[i]/count[i];
        }
    }

    // for (int i = 0; i < numCaps; i++){
    //     for(int j = 0; j < count[i]; j++) {
    //             fSum[i] += fReadings[i][j];             
    //     }
    //     fMean[i] = fSum[i] / count[i];
    // }

    Serial.print("Capacitance0: ");
    Serial.print(mean[0], 5);
    Serial.print(",");
    Serial.print(std_dev[0], 5);
    Serial.print(",");
    Serial.print(fMean[0], 5);
    Serial.print(",");
    Serial.println(count[0]);
    Serial.print("Capacitance1: ");
    Serial.print(mean[1], 5);
    Serial.print(",");
    Serial.print(std_dev[1], 5);
    Serial.print(",");
    Serial.print(fMean[1], 5);
    Serial.print(",");
    Serial.println(count[1]);
    Serial.print("Capacitance2: ");
    Serial.print(mean[2], 5);
    Serial.print(",");
    Serial.print(std_dev[2], 5);
    Serial.print(",");
    Serial.print(fMean[2], 5);
    Serial.print(",");
    Serial.println(count[2]);
    Serial.print("Capacitance3: ");
    Serial.print(mean[3], 5);
    Serial.print(",");
    Serial.print(std_dev[3], 5);
    Serial.print(",");
    Serial.print(fMean[3], 5);
    Serial.print(",");
    Serial.println(count[3]);
    //Serial.println(" ");
}


void setup() {
    
    Serial.begin(115200);
    while (!Serial) {
    ; // Wait for serial port to connect. Needed for native USB port only
    }
    Serial.println("starting");
    Wire1.setSDA(SDA1); 
    Wire1.setSCL(SCL1);
    Wire.begin();
    Wire1.begin();

    sensor0_init();
    sensor1_init();

    pinMode(Pin3_AD0, INPUT_PULLUP);
    pinMode(Pin2_AD1, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(Pin3_AD0), dataReadyISR_0, FALLING);
    attachInterrupt(digitalPinToInterrupt(Pin2_AD1), dataReadyISR_1, FALLING);

  // Interval in unsigned long microseconds
  if (ITimer.attachInterruptInterval(TIMER_INTERVAL_MS * 1000, TimerHandler))
    Serial.println("Starting ITimer OK, millis() = " + String(millis()));
  else
    Serial.println("Can't set ITimer. Select another freq. or timer");

}
   




void loop() {

    //noInterrupts(); // 
    bool isDataReady_0 = newDataAvailable_0;
    bool isDataReady_1 = newDataAvailable_1;
    bool start_conversion=TimeToStartConversion;
    //interrupts(); //  restart interrupts

    uint32_t temp=0;
    
    int cap_num = reading_count % numCaps;
    int read_num = reading_count / numCaps;


    if (Serial.available() > 0) {
        int incomingValue = Serial.parseInt();
        while (Serial.available()) {
            Serial.read(); 
        }
        dacValue = incomingValue;  // (8 pF / 0.133858 pF per unit) ≈ 60 12pF
        capSensor0.writeCapDacARegister(dacValue | AD7746_DACAEN);
        capSensor1.writeCapDacARegister(dacValue | AD7746_DACAEN);
        Serial.print("Offset changed to ");
        Serial.print(incomingValue);
    }

    // Serial.println("working");
    

    // uint32_t s = capSensor0.readStatusRegister();
    // Serial.println(s);
    
    // total += 1;
   
    if (reading_count >= numCaps * numReadings){
        // Enough readings for data processing
        //  Serial.printf("2D Array Elements:\n");
        // for (int i = 0; i < numCaps; i++) {
        //     for (int j = 0; j < numReadings; j++) {
        //         Serial.printf("%f ", readings[i][j]);
        //     }
        //     Serial.printf("\n");
        // }

        readFilteredCapacitance();
        total += reading_count;
        reading_count = 0;
    }

    if(start_conversion){
        TimeToStartConversion=false;
      //  Serial.println("Starting conversion");
        switch (cap_num){

            case 0:
                capSensor1.writeCapSetupRegister(AD7746_CAPEN );
                capSensor1.writeConfigurationRegister(AD7746_CAPF_62P0 | AD7746_MD_SINGLE_CONVERSION);
                break;
            case 1:
                capSensor0.writeCapSetupRegister(AD7746_CAPEN| AD7746_CIN2);
                capSensor0.writeConfigurationRegister(AD7746_CAPF_62P0 | AD7746_MD_SINGLE_CONVERSION);
                break;
            case 2:
                capSensor1.writeCapSetupRegister(AD7746_CAPEN| AD7746_CIN2);
                capSensor1.writeConfigurationRegister(AD7746_CAPF_62P0 | AD7746_MD_SINGLE_CONVERSION);
                break;
            case 3:
                capSensor0.writeCapSetupRegister(AD7746_CAPEN );
                capSensor0.writeConfigurationRegister(AD7746_CAPF_62P0 | AD7746_MD_SINGLE_CONVERSION);
                break;

        }
        //Serial.printf("reading count %d : %d : %d \n",reading_count,cap_num,read_num);
        
        // capSensor0.writeCapSetupRegister(0);
        // capSensor1.writeCapSetupRegister(AD7746_CAPEN);
        reading_count += 1;
    }
    if (isDataReady_0){
       // Serial.println("data0_ready");
        temp = capSensor0.getCapacitance();
        readings[cap_num][read_num] = (float) temp * (4.00 / 0x800000) + 0.133858 * dacValue;
       // readSingleCapacitance(); 
        
        newDataAvailable_0 = false;
        
        // uint32_t status = capSensor0.readStatusRegister();
        // Serial.print(status);
        // Serial.println("A");
        // Serial.println(reading_count);


    }
    if (isDataReady_1){
     //   Serial.println("data1_ready");
        temp = capSensor1.getCapacitance();
        readings[cap_num][read_num] = (float) temp * (4.00 / 0x800000) + 0.133858 * dacValue;
    //     readSingleCapacitance(); 
    //     reading_count += 1;
        newDataAvailable_1 = false;
        // uint32_t status = capSensor1.readStatusRegister();
        // Serial.print(status);   
        // Serial.print("B");
        // Serial.println(reading_count);
    }
}
