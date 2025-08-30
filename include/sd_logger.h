#pragma once
#include <Arduino.h>

bool sd_init(uint8_t csPin);
bool sd_writeHeader(const char* filename, const char* header);
bool sd_append(const char* filename, const String& data);