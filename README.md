# ESP32 Sensor Data Logger with FreeRTOS

This project demonstrates how to log environmental data from a **DHT22 temperature sensor** and a **BMP280 pressure sensor** onto an SD card using the **ESP32** and **FreeRTOS**.  

The system is designed around real-time multitasking principles:
- **Sensor tasks** periodically read data from the sensors.
- **A logger task** consumes this data from a FreeRTOS queue and writes it to both Serial and an SD card in CSV format.
- **Mutexes** ensure safe access to shared resources such as the SD card and sensors.

---

## How it works

- **DHT22 task**: Reads temperature at fixed intervals.
- **BMP280 task**: Reads pressure at fixed intervals.
- **Shared data** is protected with a mutex to avoid race conditions.
- **Logger task**:  
  - Pulls sensor data from the queue.  
  - Prints formatted readings to the Serial console.  
  - Appends entries to `/data.csv` on the SD card.  

The CSV file includes:
Time_ms,Temp_C,Pressure_hPa

where:
- `Time_ms` = timestamp from `millis()` since startup  
- `Temp_C` = temperature in °C  
- `Pressure_hPa` = pressure in hectopascals  

---

## Features

- **FreeRTOS-based design** with tasks, queues, and mutexes.  
- **Producer-consumer pattern** ensures decoupling between sensor sampling and logging.  
- **Thread-safe logging** to prevent SD corruption.  
- **Scalable**: sampling intervals and queue size can be tuned easily.  

---

## Hardware Setup

- **ESP32** development board  
- **DHT22** connected to GPIO 4  
- **BMP280** connected over I²C (address `0x76`)  
- **MicroSD card module** connected over SPI (`CS` = GPIO 5)  

---

## Example Serial Output

- [Logger] Temp=27.50 °C, Pressure=1008.23 hPa
- [Logger] Temp=27.51 °C, Pressure=1008.19 hPa

And the corresponding SD card `data.csv`:

Time_ms,Temp_C,Pressure_hPa
- 3000,27.50,1008.23
- 6000,27.51,1008.19
- 9000,27.50,1008.22

---

## Applications

- Environmental monitoring  
- Data logging for research or testing setups  
- Demonstration of multitasking and synchronization in embedded systems

---

## Circuit diagram

<img width="757" height="440" alt="Screenshot 2025-09-13 121641" src="https://github.com/user-attachments/assets/0f7f1850-18a6-4d9b-a0c1-2e02046a8a28" />

