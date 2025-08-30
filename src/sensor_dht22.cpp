#include <Arduino.h>
#include <DHT.h>
#include "sensor_dht22.h"

#define DHTTYPE DHT22
static DHT dht(4, DHTTYPE);  // default pin 4 unless re-init

void dht22_init(uint8_t pin) {
    dht = DHT(pin, DHTTYPE);
    dht.begin();
}

float dht22_getTemperature() {
    return dht.readTemperature();
}

float dht22_getHumidity() {
    return dht.readHumidity();
}
