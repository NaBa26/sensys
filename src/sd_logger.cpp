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
#define QUEUE_SIZE 20

struct SensorData {
    unsigned long time_ms;
    float temp;
    float pressure;
};

static QueueHandle_t dataQueue;
static SemaphoreHandle_t serialMutex;

File logFile;
static int entryCount = 0;

static void safePrint(const char* format, ...) {
    if (xSemaphoreTake(serialMutex, pdMS_TO_TICKS(50))){
        va_list args;
        va_start(args, format);
        char buffer[256];
        vsnprintf(buffer, sizeof(buffer), format, args);
        Serial.println(buffer);
        va_end(args);
        xSemaphoreGive(serialMutex);
    }
}

static void sensorTask(void *pvParameters) {
    const UBaseType_t uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
    safePrint("Space remaining before the task has stated: %d", uxHighWaterMark);

    TickType_t lastWake = xTaskGetTickCount();

    while (true) {
        SensorData data;
        data.time_ms = millis();
        data.temp = dht22_getTemperature();
        data.pressure = bmp280_getPressure();

        if (xQueueSend(dataQueue, &data, pdMS_TO_TICKS((100)) != pdTRUE)) {
            safePrint("Queue full! Data dropped");
        }
        safePrint("Space remaining after the data has been sent to the queue: %d", uxHighWaterMark);

        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(2000)); // every 2s
    }
}

// Logs queued data to SD
static void loggerTask(void *pvParameters) {
    while (true) {
        SensorData data;

        if (xQueueReceive(dataQueue, &data, portMAX_DELAY)) {
                if (entryCount == 0) {
                    logFile = SD.open("/data.csv", FILE_WRITE);
                    if (logFile) {
                        logFile.println("Time_ms,Temp_C,Pressure_hPa");
                        logFile.flush(); //to ensure that the header is written
                        safePrint("Started a new log file here.");
                    } else {
                        safePrint("Failed to open the log file.");
                        continue;
                    }
                }

                if (logFile) {
                    logFile.printf("%lu,%.2f,%.2f\n",
                                   data.time_ms,
                                   data.temp,
                                   data.pressure);
                    // flushing only periodically
                    if (entryCount%10==0) {
                        logFile.flush();
                    }
                    entryCount++;
                }

            safePrint("[Logger] Temp=%.2f °C, Pressure=%.2f hPa\n",
                          data.temp, data.pressure);

                if (entryCount >= MAX_ENTRIES) {
                    logFile.flush();
                    logFile.close();
                    SD.rename("/data.csv", "/data_old.csv");

                    entryCount = 0;
                    safePrint("Log file rotated.");
                }
        }
    }
}

void startLogger() {
    //normal Serial.println() works here cause only the main thread is running
    Serial.begin(115200);

    if (!SD.begin(SD_CS)) {
        Serial.println("SD init failed!\n");
        while (true) {
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }
    Serial.println("SD card initialized\n");

    dht22_init(DHT_PIN);
    bmp280_init();
    Serial.println("Sensors initialized\n");


    dataQueue = xQueueCreate(10, sizeof(SensorData));
    serialMutex = xSemaphoreCreateMutex();

    if (!dataQueue || !serialMutex) {
        Serial.println("Failed to create queue/semaphores!\n");
        while (true) vTaskDelay(pdMS_TO_TICKS(1000));
    }

    xTaskCreate(sensorTask, "Sensor Task", 2048, NULL, 3,  NULL );
    xTaskCreate(loggerTask, "Logger Task", 2048, NULL, 2,  NULL );

    Serial.println("Tasks created successfully.\n");
}
