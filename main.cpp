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
#define LED_RED    PB_14  // Red
#define LED_GREEN  PB_1  // Green 
#define LED_BLUE   PB_15  // Blue 
#define PUMP_PIN   PC_1    // Relay for Nutrient Solution
#define TRIG_PIN   PA_3  // Ultrasonic Sensor TRIG pin
#define ECHO_PIN   PA_2  // Ultrasonic Sensor ECHO pin
#define BUZZER_PIN PA_1  // Buzzer for alerts
#define SERVO_PIN  PA_7    // Servo Motor PWM pin
#define bd_btn1    PA_15   // Button 1
#define BUTTON2    PC_12   // Button 2
#define LDR_DO     PD_2    //LDR_Digital Output
#define LDR_AO     PA_0    //LDR_Analog Output
#define FAN        PA_3    //Fan Output to cool plants
//#define BUTTON3 D12      // Button 3

//  Initializing Components 
DHT11 dht(DHTPIN);
DigitalOut fan(FAN);
DigitalOut red(LED_RED);
DigitalOut green(LED_GREEN);
DigitalOut blue(LED_BLUE);
DigitalOut pump(PUMP_PIN);
PwmOut buzzer(BUZZER_PIN);
DigitalOut trig(TRIG_PIN);
DigitalIn echo(ECHO_PIN);
InterruptIn button1(bd_btn1);
InterruptIn button2(BUTTON2);
//LDR Init
DigitalIn LDRSensor_DO(LDR_DO);
AnalogIn LDRSensor_AO(LDR_AO);
//DigitalIn button3(BUTTON3);
PwmOut servo(SERVO_PIN);
Timer timer;

// Create a serial object for USART communication (using PB6 for TX and PB7 for RX)
UnbufferedSerial serial(PB_6, PB_7); // Define the serial port for PB6 (TX) and PB7 (RX)
int buttonPressed = 0;
int temperature = 0;
int humidity = 0;
int raw_adc_value;

// Servo Motor Rotation  
int clockwise = 2000;  
int anti_clockwise = 1000;  
float stop = 1500;  

//Repeating Functions
//  Function for RGB LED 
void setRGB(bool r, bool g, bool b) {
    red = r;
    green = g;
    blue = b;
}
// Function for Servo Motor 
void move_servo(int pulse_width) { 
    servo.pulsewidth_us(pulse_width); 
}
// Declare protype height to calculate plant height 
int total_height = 8; //8cm
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
    printf("\nTemperature: %2d", temperature);
    printf("\nHumidity: %2d\n", humidity);
    
    if (result == 0) { // If reading was successful
        // Send the temperature and humidity data as plain strings
        char tempBuffer[50];
        char humidityBuffer[50];
        
        // Format the temperature and humidity values into a buffer
        int tempLength = sprintf(tempBuffer, "Temperature: %d C\n", temperature);
        int humidityLength = sprintf(humidityBuffer, "Humidity: %d %%\n", humidity);
        
        // Send the data to the Raspberry Pi via serial
        serial.write(tempBuffer, tempLength);  // Send temperature
        serial.write(humidityBuffer, humidityLength);  // Send humidity
    } else { // If there was an error
        char errorBuffer[50];
        int errorLength = sprintf(errorBuffer, "Error: %s\n", dht.getErrorString(result));
        serial.write(errorBuffer, errorLength);  // Send error message
    }
}
float adcToLux() {
    float adc_value = LDRSensor_AO.read();  
    raw_adc_value = adc_value * 3914;

    // Prevent divide by zero error
    if (raw_adc_value > 3000) {
        return 0.0f;  // Assume no light
    }

    // Convert ADC value to lux using the empirical formula
    int lux = A_CONST * pow(raw_adc_value, -B_CONST);
    
    return lux;
}
// LDR Code
void LDR_USART(){
    float adc_value = LDRSensor_AO.read();  
    int lux = adcToLux();
    char luxBuffer[50];

    // Convert the ADC value to raw (0 to 4095)
    raw_adc_value = adc_value * 3914;
    int voltage = adc_value * VREF;
    int luxLength = sprintf(luxBuffer, "Lux: %d\n", lux);                       // Format lux data
    serial.write(luxBuffer, luxLength);  // Send lux value

    // Print the result
    printf("ADC Value: %d,\n Voltage: %d\n", raw_adc_value, lux);
    if (raw_adc_value >= 2600) { // Change value to User set value -- need research
        setRGB(1, 0, 1); // Purple LED when light is too high
    } else {
        setRGB(0, 0, 0); // Turn off LED when it's dark enough
    }
}

// Sensor Check Thread (for buzzer logic)
void sensorCheckThread() {
    while(true) {
        if (temperature > 27 && raw_adc_value >= 2600) {
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
    TempHumiUSART();
    LDR_USART();
    ThisThread::sleep_for(5000ms);
    }
}
void button1_pressed() {
    //printf("Button 1 Pressed: Moving Servo Clockwise\n");
    setRGB(1, 0, 0);
    buttonPressed=1;
}
void button2_pressed() {
    //printf("Button 1 Pressed: Moving Servo Clockwise\n");
    setRGB(1, 0, 0);
    buttonPressed=2;
}
Thread servoControlThread;
void servoControl() {
    while (true) {

        switch (buttonPressed) {
            case 1:
                printf("Button 1 Pressed: LED Red, Previous Bay\n");
                move_servo(anti_clockwise);
                ThisThread::sleep_for(5s);
                buttonPressed = 0;
                break;

            case 2:
                printf("Button 2 Pressed: LED Green, Next Bay\n");
                move_servo(clockwise);
                ThisThread::sleep_for(5s);
                buttonPressed = 0;
                break;

            default:
                move_servo(stop);
                break;
        }
        ThisThread::sleep_for(100ms);
    }
}

int main() {
    // Initialize serial communication (set the baud rate to match the Raspberry Pi)
    serial.baud(9600);  // Ensure the baud rate matches
    serial.format(8, SerialBase::None, 1);  // 8 data bits, no parity, 1 stop bit
    buzzer.period(1.0/440.0);
    button1.fall(&button1_pressed);  // Detect falling edge (button press)
    button2.fall(&button2_pressed);  // Detect falling edge (button press)

    // Wait for the serial to be ready before starting
    uSARTsendThread.start(USARTsendThread);
    // Start sensor check thread
    Thread SensorCheckThread;
    SensorCheckThread.start(sensorCheckThread);
    while(true){
        servoControl();
    }
}
