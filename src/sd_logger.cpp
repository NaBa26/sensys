#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include "sd_logger.h"

static File dataFile;

bool sd_init(uint8_t csPin) {
    if (!SD.begin(csPin)) {
        Serial.println("SD init failed!");
        return false;
    }
    Serial.println("SD Card initialized.");
    return true;
}

bool sd_writeHeader(const char* filename, const char* header) {
    dataFile = SD.open(filename, FILE_WRITE);
    if (dataFile) {
        if (dataFile.size() == 0) {
            dataFile.println(header);
        }
        dataFile.close();
        return true;
    }
    return false;
}

bool sd_append(const char* filename, const String& data) {
    dataFile = SD.open(filename, FILE_WRITE);
    if (dataFile) {
        dataFile.println(data);
        dataFile.close();
        return true;
    }
    return false;
}