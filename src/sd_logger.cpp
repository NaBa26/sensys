#include <Arduino.h>
#include <SD.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>

#include "sensor_dht22.h"
#include "sensor_bmp280.h"

#define SD_CS       5      // change if you hit strapping-pin issues
#define DHT_PIN     4
#define QUEUE_SIZE  50     // larger buffer to tolerate short SD stalls
#define FLUSH_EVERY 10     // flush to SD every N writes

struct SensorData {
    unsigned long time_ms;
    float temp;
    float pressure;
};

static QueueHandle_t dataQueue = NULL;
static SemaphoreHandle_t serialMutex = NULL;

static File logFile;
static int entryCount = 0;

// --- helpers ---------------------------------------------------------------

static void safePrint(const char* format, ...) {
    if (xSemaphoreTake(serialMutex, pdMS_TO_TICKS(200))) {
        va_list args;
        va_start(args, format);
        char buffer[256];
        vsnprintf(buffer, sizeof(buffer), format, args);
        Serial.println(buffer);
        va_end(args);
        xSemaphoreGive(serialMutex);
    }
}

// Open /data.csv and write header if file is empty or new
static bool openLogFile() {
    logFile = SD.open("/data.csv", FILE_WRITE); // truncates existing file
    if (!logFile) {
        safePrint("Failed to open /data.csv for writing!");
        return false;
    }

    logFile.println("Time_ms,Temp_C,Pressure_hPa");
    safePrint("Opened /data.csv and wrote header (file handle=%p)", &logFile);
    return true;
}

// --- tasks -----------------------------------------------------------------

static void sensorTask(void *pvParameters) {
    TickType_t lastWake = xTaskGetTickCount();
    const TickType_t interval = pdMS_TO_TICKS(2000); // sensor sample every 2s

    for (;;) {
        SensorData sample;
        sample.time_ms = millis();
        sample.temp = dht22_getTemperature();
        sample.pressure = bmp280_getPressure();

        if (xQueueSend(dataQueue, &sample, pdMS_TO_TICKS(500)) != pdTRUE) {
            safePrint("Queue full! Dropped sample at %lu", sample.time_ms);
        }

        vTaskDelayUntil(&lastWake, interval);
    }
}

static void loggerTask(void *pvParameters) {
    if (!openLogFile()) {
        safePrint("LoggerTask: cannot open log file, aborting task.");
        vTaskDelete(NULL);
    }

    entryCount = 0;

    SensorData received;
    for (;;) {
        if (xQueueReceive(dataQueue, &received, portMAX_DELAY) == pdTRUE) {
            if (!logFile) {
                safePrint("LoggerTask: file handle invalid");
                while (true) vTaskDelay(pdMS_TO_TICKS(1000));
            }

            logFile.printf("%lu,%.2f,%.2f\n",
                           received.time_ms,
                           received.temp,
                           received.pressure);
            entryCount++;

            if ((entryCount % FLUSH_EVERY) == 0) {
                logFile.flush();
                safePrint("Flushed after %d entries", entryCount);
            }

            safePrint("Logged entry %d -> /data.csv : %lu, %.2f, %.2f",
                      entryCount, received.time_ms, received.temp, received.pressure);
        }
    }
}

void startLogger() {
    Serial.begin(115200);
    vTaskDelay(pdMS_TO_TICKS(10));

    if (!SD.begin(SD_CS)) {
        Serial.println("SD init failed! Halting.");
        while (true) vTaskDelay(pdMS_TO_TICKS(1000));
    }
    Serial.println("SD initialized");

    dht22_init(DHT_PIN);
    bmp280_init();
    Serial.println("Sensors initialized");

    dataQueue = xQueueCreate(QUEUE_SIZE, sizeof(SensorData));
    serialMutex = xSemaphoreCreateMutex();

    if (!dataQueue || !serialMutex) {
        Serial.println("Failed to create queue or mutex; halting.");
        while (true) vTaskDelay(pdMS_TO_TICKS(1000));
    }

    xTaskCreate(loggerTask, "Logger", 4096, NULL, 3, NULL);
    xTaskCreate(sensorTask, "Sensor", 4096, NULL, 2, NULL);

    safePrint("Tasks created: Logger (prio 3), Sensor (prio 2). Queue size=%d", QUEUE_SIZE);
}
