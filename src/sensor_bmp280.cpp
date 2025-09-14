#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_BMP280.h>
#include "sensor_bmp280.h"
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

static Adafruit_BMP280 bmp;
static SemaphoreHandle_t bmpMutex;

void bmp280_init() {
    if (!bmp.begin(0x76)) {
        Serial.println("BMP280 not found! Check wiring.");
        while (true) {
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
    }
    bmpMutex = xSemaphoreCreateMutex();
}

float bmp280_getPressure() {
    float pressure = NAN;
    if (xSemaphoreTake(bmpMutex, pdMS_TO_TICKS(200))) {
        pressure = bmp.readPressure() / 100.0;
        xSemaphoreGive(bmpMutex);
    }
    return pressure;
}