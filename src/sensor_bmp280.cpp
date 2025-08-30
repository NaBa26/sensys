#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_BMP280.h>
#include "sensor_bmp280.h"

Adafruit_BMP280 bmp;

void bmp280_init() {
    if (!bmp.begin(0x76)) {
        Serial.println("BMP280 not found! Check wiring.");
        while (1);
    }
}

float bmp280_getTemperature() {
    return bmp.readTemperature();
}

float bmp280_getPressure() {
    return bmp.readPressure() / 100.0;  // hPa
}

float bmp280_getAltitude(float seaLevelhPa) {
    return bmp.readAltitude(seaLevelhPa);
}






