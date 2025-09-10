#include <Arduino.h>
#include <SD.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>

#include "sensor_dht22.h"
#include "sensor_bmp280.h"

#define SD_CS 5
#define MAX_ENTRIES 5000
#define DHT_PIN 4

struct SensorData {
    unsigned long time_ms;
    float temp;
    float pressure;
};

static QueueHandle_t dataQueue;
static SemaphoreHandle_t sdMutex;
static SemaphoreHandle_t serialMutex;

File logFile;
static int entryCount = 0;

// Safe serial print
static void safePrint(const char* msg) {
    if (xSemaphoreTake(serialMutex, pdMS_TO_TICKS(50))) {
        Serial.println(msg);
        xSemaphoreGive(serialMutex);
    }
}

// ----------- Tasks ------------

// Reads sensors and pushes to queue
static void sensorTask(void *pvParameters) {
    TickType_t lastWake = xTaskGetTickCount();
    while (true) {
        SensorData data;
        data.time_ms = millis();
        data.temp = dht22_getTemperature();
        data.pressure = bmp280_getPressure();

        xQueueSend(dataQueue, &data, portMAX_DELAY);

        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(3000)); // every 3s
    }
}

// Logs queued data to SD
static void loggerTask(void *pvParameters) {
    while (true) {
        SensorData data;
        if (xQueueReceive(dataQueue, &data, portMAX_DELAY)) {
            if (xSemaphoreTake(sdMutex, portMAX_DELAY)) {
                if (entryCount == 0) {
                    logFile = SD.open("/data.csv", FILE_WRITE);
                    if (logFile) {
                        logFile.println("Time_ms,Temp_C,Pressure_hPa");
                    }
                }

                if (logFile) {
                    logFile.printf("%lu,%.2f,%.2f\n",
                                   data.time_ms,
                                   data.temp,
                                   data.pressure);
                    logFile.flush();
                    entryCount++;
                }

                if (entryCount >= MAX_ENTRIES) {
                    logFile.close();
                    SD.remove("/data.csv");
                    entryCount = 0;
                }

                xSemaphoreGive(sdMutex);
            }

            // Mirror to serial for debug
            Serial.printf("[Logger] Temp=%.2f °C, Pressure=%.2f hPa\n",
                          data.temp, data.pressure);
        }
    }
}

// ----------- Init ------------
void startLogger() {
    if (!SD.begin(SD_CS)) {
        Serial.println("SD init failed!");
        while (true) {
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
    }

    dht22_init(DHT_PIN);
    bmp280_init();

    dataQueue = xQueueCreate(10, sizeof(SensorData));
    sdMutex = xSemaphoreCreateMutex();
    serialMutex = xSemaphoreCreateMutex();

    xTaskCreatePinnedToCore(sensorTask, "Sensor Task", 4096, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(loggerTask, "Logger Task", 4096, NULL, 1, NULL, 1);
}
