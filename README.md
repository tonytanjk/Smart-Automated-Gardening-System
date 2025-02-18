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
    - Flask
    - PySerial
    - SQLite3 (for local database management)
    - HTML/CSS/JavaScript for frontend

### Install Dependencies

Run the following commands to install the required Python libraries ( in a python virtual environmnet):
Python Virtual Environment: https://www.raspberrypi.com/documentation/computers/os.html#python-on-raspberry-pi
```bash
pip install flask
pip install pyserial
pip install sqlite3
pip install flask-socketio
