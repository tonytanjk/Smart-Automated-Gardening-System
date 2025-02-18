#include <cstdio>
#undef __ARM_FP
#include "mbed.h"
#include "DHT11.h"
#include "lcd.h"
#include "keypad.h"
#define WAIT_TIME_MS_0 100
#define WAIT_TIME_MS_1 50
#define VREF 3.3f  // Reference Voltage for LDR

#define MAX_ADC 4095.0f  // 12-bit ADC range
#define A_CONST 500000.0f  // Experimentally determined constant
#define B_CONST 1.0f  // Experimentally determined constant

// Sensor & Actuator Pin Assignments
#define DHTPIN     PA_4    // DHT11 Pin
#define LED_RED    PB_0  // Red
#define LED_GREEN  PB_1  // Green 
#define LED_BLUE   PB_2  // Blue 
#define TRIG_PIN   PA_3  // Ultrasonic Sensor TRIG pin
#define ECHO_PIN   PA_2  // Ultrasonic Sensor ECHO pin
#define BUZZER_PIN PA_1  // Buzzer for alerts
#define SERVO_Reverse_PIN  PA_6    // Servo Motor PWM pin
#define SERVO_PIN  PA_7    // Servo Motor PWM pin
#define bd_btn1    PA_15   // Button 1
#define BUTTON2    PC_12   // Button 2
#define LDR_DO     PD_2    //LDR_Digital Output
#define LDR_AO     PA_0    //LDR_Analog Output
#define FAN        PA_3    //Fan Output to cool plants
#define pH         PC_1    // pH Sensor Output

//  Initializing Components 
DHT11 dht(DHTPIN);
DigitalOut fan(FAN);
DigitalOut red(LED_RED);
DigitalOut green(LED_GREEN);
DigitalOut blue(LED_BLUE);
PwmOut buzzer(BUZZER_PIN);
DigitalOut trig(TRIG_PIN);
DigitalIn echo(ECHO_PIN);
InterruptIn button1(bd_btn1);
InterruptIn button2(BUTTON2);
//LDR Init
DigitalIn LDRSensor_DO(LDR_DO);
AnalogIn LDRSensor_AO(LDR_AO);
AnalogIn pHSensor(pH);
//DigitalIn button3(BUTTON3);
PwmOut servo(SERVO_PIN);
PwmOut reverse_servo(SERVO_Reverse_PIN);

Timer timer;
Mutex serialWriteMutex;
Mutex serialReadMutex;

// Create a serial object for USART communication (using PB6 for TX and PB7 for RX)
UnbufferedSerial serial(PB_6, PB_7); // Define the serial port for PB6 (TX) and PB7 (RX)
int buttonPressed = 0;
int temperature = 0;
int humidity = 0;
int lux = 0;
int pHValue = 0;
int raw_adc_value;

// Servo Motor Rotation  
bool movingForward = false;  // To track if forward movement is triggered
bool movingBackward = false;  // To track if reverse movement is triggered

//Repeating Functions
//  Function for RGB LED 
void setRGB(bool r, bool g, bool b) {
    red = r;
    green = g;
    blue = b;
}

// Function for Servo Motor 
void move_forward() {
    servo.pulsewidth(0.001);  // Forward direction for left wheel
    reverse_servo.pulsewidth(0.003); // Forward direction for right wheel
    ThisThread::sleep_for(250ms);  // Move for a short time
    servo.pulsewidth(0.0015); // Stop left wheel (neutral)
    reverse_servo.pulsewidth(0.0015); // Stop right wheel (neutral)
}

void move_backward() {
    servo.pulsewidth(0.003);  // Reverse direction for left wheel
    reverse_servo.pulsewidth(0.001); // Reverse direction for right wheel
    ThisThread::sleep_for(250ms);  // Move for a short time
    servo.pulsewidth(0.0015); // Stop left wheel (neutral)
    reverse_servo.pulsewidth(0.0015); // Stop right wheel (neutral)
}

// Function to Read Ultrasonic Sensor (plant height) 
float getUltrasonicDistance() {
    trig = 0; //trig low 
    wait_us(2);
    trig = 1; //trig high 
    wait_us(10);
    trig = 0; //trig low 

    timer.start();
    while (echo == 0);  // Wait for echo signal to start
    timer.reset();
    while (echo == 1);  // Measure how long the echo signal is HIGH
    timer.stop();

    float duration = timer.elapsed_time().count();
    return (duration/2) / 29.1;  // Convert to cm
}

// Temperature and Humidity Code
void TempHumiUSART(){
    int result = dht.readTemperatureHumidity(temperature, humidity);
    if (result == 0) { // If reading was successful
        char tempBuffer[50];
        char humidityBuffer[50];
        
        int tempLen = sprintf(tempBuffer, "Temperature: %d C\n", temperature);
        int humidityLen = sprintf(humidityBuffer, "Humidity: %d %%\n", humidity);
        serial.write(tempBuffer, tempLen);
        serial.write(humidityBuffer, humidityLen);
    }
}
float adcToLux() {
    float adc_value = LDRSensor_AO.read();  
    raw_adc_value = adc_value * 4095;


    // Convert ADC value to lux using the empirical formula
    int lux = A_CONST * pow(raw_adc_value, -B_CONST);
    
    return lux;
}
// LDR Code
void LDR_USART(){
    float adc_value = LDRSensor_AO.read();  
    int lux = adcToLux();
    char luxBuffer[50];
    int luxLength = sprintf(luxBuffer, "Lux: %d\n", lux);                       
    serial.write(luxBuffer, luxLength);  
}

// pH Sensor Code
float read_ph_level() {
    float voltage = pHSensor.read() * 3.3;  
    float pHValue = (voltage * 14.0) / 3.3;  // (needs calibration)
    return pHValue;
}
void pH_USART(){
    char pHBuffer[50];
    int pHValue = read_ph_level();
    int pHLength = sprintf(pHBuffer, "pH: %d\n", pHValue);                       
    serial.write(pHBuffer, pHLength);  
}

// Sensor Check Thread (for buzzer logic)
Thread SensorCheckThread;
void sensorCheckThread() {
    while(true) {
        if (temperature > 27 && lux <= 200) {
            buzzer = 0.5;              // 50% duty cycle: the buzzer sounds at 440Hz
            thread_sleep_for(500);     // Sound for 500ms
            buzzer = 0.0;              // No output: silent
            thread_sleep_for(50);     // Silence for 500ms
        } else {
            buzzer = 0; // No buzzer if conditions are not met
        }
        ThisThread::sleep_for(1000ms); // Periodic check delay
    }
}

Thread uSARTsendThread;
void USARTsendThread(){
    while(true){
        serialWriteMutex.lock();  // Lock serial write access
        TempHumiUSART();
        LDR_USART();
        pH_USART();
        serialWriteMutex.unlock();  // Unlock serial write access
        ThisThread::sleep_for(5000ms);
    }
}

void button1_pressed() {
    setRGB(1, 0, 0);  // Red LED to indicate moving forward
    movingForward = true;
}

void button2_pressed() {
    setRGB(0, 1, 0);  // Blue LED to indicate moving backward
    movingBackward = true;
}

// Handle USART commands (for remote control)
Thread UsartCommandThread;
void UsartCommandRPI() {
    char command;
    serial.write("STM32 USART Ready...\n", 21);

    while (true) {
        // Check if data is available to read (non-blocking)
        if (serial.readable()) {
            serialReadMutex.lock();  // Lock serial read access
            int numBytesRead = serial.read(&command, 1);  // Read a single byte
            serialReadMutex.unlock();  // Unlock serial read access

            if (numBytesRead > 0) {
                printf("Received: %c\n", command);
                // Respond to commands
                if (command == 'F') {
                    movingForward = true;
                } else if (command == 'B') {
                    movingBackward = true;
                }
            }
        }
        ThisThread::sleep_for(10ms);  // Small delay to prevent 100% CPU usage
    }
}

void servoControl() {
    servo.period(0.15);  // 20ms period for servo control
    reverse_servo.period(0.15);

    while (true) {
        if (movingForward) {
            move_forward();
            movingForward = false;
        } else if (movingBackward) {
            move_backward();
            movingBackward = false;
        }
    }
}

Thread usartThread;
void UsartThreadFunction() {
    char command;
    //serial.write("STM32 USART Ready...\n", 21);
    while (true) {
        // Check if data is available to read (non-blocking)
        if (serial.readable()) {
            serialReadMutex.lock();  // Lock serial read access
            int numBytesRead = serial.read(&command, 1);  // Read a single byte
            serialReadMutex.unlock();  // Unlock serial read access

            if (numBytesRead > 0) {
                printf("Received: %c\n", command);
                // Respond to commands
                if (command == 'F') {
                    movingForward = true;
                } else if (command == 'B') {
                    movingBackward = true;
                }
            }
        }        
        // Regularly send sensor data
        serialWriteMutex.lock();  // Lock serial write access
        TempHumiUSART();
        LDR_USART();
        pH_USART();
        serialWriteMutex.unlock();  // Unlock serial write access
        ThisThread::sleep_for(1s);
    }

    }

// Main entry point
int main() {
                buzzer = 0.5;              // 50% duty cycle: the buzzer sounds at 440Hz
            thread_sleep_for(500);     // Sound for 500ms
            buzzer = 0.0;  
    // Initialize serial communication
    serial.baud(9600);  // Ensure the baud rate matches
    serial.format(8, SerialBase::None, 1);  // 8 data bits, no parity, 1 stop bit
    buzzer.period(1.0 / 440.0);
    button1.fall(&button1_pressed);  // Detect falling edge (button press)
    button2.fall(&button2_pressed);  // Detect falling edge (button press)

    // Start sensor check thread
    SensorCheckThread.start(sensorCheckThread);
    
    // Start USART send thread
    //uSARTsendThread.start(USARTsendThread);
    usartThread.start(UsartThreadFunction);

    // Start USART command thread
    //UsartCommandThread.start(UsartCommandRPI);

    // Start servo control in main loop
    servoControl();
}
