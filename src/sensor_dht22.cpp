#include <Arduino.h>
#include <DHT.h>
#include "sensor_dht22.h"
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

#define DHT_TYPE DHT22
static DHT dht(4, DHT_TYPE);
static SemaphoreHandle_t dhtMutex;

void dht22_init(uint8_t pin) {
    dht = DHT(pin, DHT_TYPE);
    dht.begin();
    dhtMutex = xSemaphoreCreateMutex();
}

float dht22_getTemperature() {
    float temp = NAN;
    if (xSemaphoreTake(dhtMutex, pdMS_TO_TICKS(200))) {
        temp = dht.readTemperature();
        xSemaphoreGive(dhtMutex);
    }
    return temp;
}
