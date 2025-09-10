#include <Arduino.h>
#include "sd_logger.h"

void setup() {
    Serial.begin(115200);
    startLogger();
}

void loop() {
    // Nothing — FreeRTOS runs tasks
}
