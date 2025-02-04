#undef __ARM_FP
// Declaration of all Dependency Files for Code Functionality
#include "mbed.h"
#include "DHT.h"
#include "keypad.h"
#include "lcd.h"
#include "SRF05.h"

// Definition of All Pins used for project 
#define DHT_PIN PA_0 //Temp Humidity Sensor Pin
#define buzz PA_1
#define Moist PA_1
// Define the pins for the SRF05 sensor
#define TRIG_PIN PA_4
#define ECHO_PIN PA_1

//______________________Keypad________________________
// Define row and column pins (Refer to Keypad Code in MAPP LAB)
#define ROW1 PA_0
#define ROW2 PA_1
#define ROW3 PA_2
#define ROW4 PA_3
#define COL1 PA_4
#define COL2 PA_5
#define COL3 PA_6
#define COL4 PA_7

// Declare row and column pins as DigitalOut and DigitalIn
DigitalOut rows[] = {DigitalOut(ROW1), DigitalOut(ROW2), DigitalOut(ROW3), DigitalOut(ROW4)};
DigitalIn cols[] = {DigitalIn(COL1), DigitalIn(COL2), DigitalIn(COL3), DigitalIn(COL4)};

// Keypad mapping
char keys[4][4] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};

// Function to scan the keypad
char scanKeypad() {
    for (int row = 0; row < 4; row++) {
        // Set all rows to HIGH
        for (int i = 0; i < 4; i++) {
            rows[i] = 1;
        }

        // Set the current row to LOW
        rows[row] = 0;

        // Check each column
        for (int col = 0; col < 4; col++) {
            if (cols[col] == 0) {  // If a button is pressed
                wait_us(20000);   // Debounce delay
                if (cols[col] == 0) {  // Confirm the button is still pressed
                    return keys[row][col];
                }
            }
        }
    }
    return '\0';  // Return null character if no key is pressed
}
//_____________________Keypad________________________

// ALL Declarations for Inputs and Outputs

//Output:
DigitalOut trig(TRIG_PIN);
DigitalOut buzzer(buzz);
AnalogIn MoistSense(Moist);

//Input:
DigitalIn echo(ECHO_PIN);
DigitalIn OnBdButton1(PA_1);
DigitalIn OnBdButton2(PA_6);
DigitalIn YellowBtn(PA_1);
DigitalIn GreenBtn(PA_1);

//Dependency Declarations
Timer timer;
DHT dht(DHT_PIN, DHT11); 

//Global Variables for code Usage
bool SenseHealth = false; //Sensor Check Pass/Fail Bool
int THCk, USCk, KPCk, MTCk; // Temperature Pass/Fail Bool


void SensorInit(){
//TODO Erika
}


void TempHumiCheck(){
    int err = dht.readData();  // Read temperature and humidity from the sensor
    if (err == ERROR_NONE) {
        printf("Temperature: %.1f°C\n", dht.ReadTemperature(CELCIUS));
        printf("Humidity: %.1f%%\n", dht.ReadHumidity());
        THCk = 1;
        } 
    else 
    {
        printf("Error: %d\n", err);
        }
    }

float readMoisture() {
    // Read the analog value (0.0 to 1.0 corresponding to 0-3.3V)
    float moistureValue = MoistSense.read();
    // Convert to percentage
    float moisturePercentage = moistureValue * 100.0;
    return moisturePercentage;
}

float measureDistance() {
    // Send a 10us pulse to trigger
    trig = 1;
    wait_us(10);  // Pulse width = 10 microseconds
    trig = 0;

    // Wait for echo to go HIGH
    while (echo.read() == 0);

    // Start the timer when echo goes HIGH
    timer.reset();
    timer.start();

    // Wait for echo to go LOW
    while (echo.read() == 1);

    // Stop the timer
    timer.stop();

    // Calculate distance in cm (duration in microseconds / 58 = distance in cm)
    float distance = timer.elapsed_time().count() / 58.0;

    return distance;
    }

void UltraSenseCheck(){
    float Ultrasonic_Dist = measureDistance();
    if(Ultrasonic_Dist == NULL || Ultrasonic_Dist < 0){
        printf("Check Ultrasonic Sensor!!");
    }
    else{
        USCk = 1;
        printf("Ultrasonic Test Passed");
    }
}

void KeyPadCheck(){
    if(KPCk == NULL || KPCk == 0 ){
    printf("KeyPad Test, Press a key");
    while(true){
    char key = scanKeypad();
    if (key != '\0') {  // If a key is detected
    printf("Key Pressed: %c\n", key);
    KPCk = 1;
    }
    break;
    }
    }
}

void MoistCheck(){
    if(MTCk == NULL || MTCk == 0 ){
        float data = readMoisture();
        if(data <0){
            printf("Check Moisture Sensor");
        }
        else{
            printf("Moisture Test Passed");
            SenseHealth = true;
            return;
        }
    }
}

void SensorHealthChk(){
    if (SenseHealth == false){
        if (THCk == 0 || THCk == NULL){ //Prevent Continuous Check if passed
            TempHumiCheck();
        }
        if(USCk == 0 || USCk == NULL){
            UltraSenseCheck();
        }
        if(KPCk == 0 || KPCk == NULL){
            KeyPadCheck();
        }
        if(MTCk == 0 || MTCk == NULL){
            MoistCheck();
        }
    }
    else{
        printf("Sensor Health Check Passed");
    }
}

void ErrorHandle(){
    //TODO
}



// main() runs in its own thread in the OS
int main()
{
    while (true) {

    }
}

