# Hydroponics Dashboard

This project provides a web-based dashboard for monitoring and controlling various environmental parameters in a hydroponic farming system. It uses Flask as a backend server and communicates with various sensors (e.g., temperature, humidity, light intensity) through a Raspberry Pi. The data from these sensors is collected via a serial connection and displayed on the dashboard in real-time.

## Features

- **Real-Time Monitoring**: View temperature, humidity, light intensity (Lux), and other system data.
- **System Overview**: Check important parameters like pH level, water level, and pump status.
- **Bay Management**: Manage and track multiple plant bays, each with its own environmental settings and plant types.
- **Automated Data Updates**: The dashboard fetches updated sensor data every few seconds.

## Setup Instructions

### Requirements

1. **Hardware**:
    - Raspberry Pi (any model with GPIO and serial support)
    - LDR (Light Dependent Resistor) sensor for light intensity
    - DHT11/DHT22 sensor for temperature and humidity
    - Other sensors (e.g., pH, water level) as required

2. **Software**:
    - Python 3.x
    - Flask (`pip install flask`)
    - PySerial (`pip install pyserial`)
    - SQLite3 (for local database management)
    - HTML/CSS/JavaScript for frontend

### Serial Communication

The system reads data from a serial-connected device (such as an Arduino or a microcontroller) via a UART (Universal Asynchronous Receiver-Transmitter) connection. The backend listens for incoming data and updates the parameters such as temperature, humidity, and light intensity.

### Database Setup

1. Install SQLite3 to store bay configurations.
2. The database stores plant bay information such as plant types, temperature ranges, pH ranges, and water levels.

### Configuration

In the Python file `app.py`, adjust the following configurations as needed:

- **Serial Port**: Modify the serial port (`/dev/ttyS0`) to match your system's configuration.
- **Baud Rate**: The default baud rate is set to 9600. Ensure that this matches the baud rate of your serial device.

### Running the Project

1. **Clone the repository** (if applicable) and navigate to the project directory.
2. Install dependencies:
   ```bash
   pip install -r requirements.txt
