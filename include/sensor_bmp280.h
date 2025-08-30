#pragma once
#include <Adafruit_BMP280.h>

void bmp280_init();
float bmp280_getTemperature();
float bmp280_getPressure();
float bmp280_getAltitude(float seaLevelhPa = 1013.25);
