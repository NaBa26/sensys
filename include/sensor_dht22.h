#pragma once
#include <DHT.h>

void dht22_init(uint8_t pin);
float dht22_getTemperature();
float dht22_getHumidity();