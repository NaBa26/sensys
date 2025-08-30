#include <Arduino.h>
#include "sensor_bmp280.h"
#include "sensor_dht22.h"
#include "sd_logger.h"

#define SD_CS 5
#define DHTPIN 4

void setup() {
    Serial.begin(115200);
    delay(2000);

    Serial.println("System starting...");

    // Init sensors
    bmp280_init();
    dht22_init(DHTPIN);

    // Init SD
    if (!sd_init(SD_CS)) {
        while (1); // stop if SD fails
    }

    // Write headers (two separate CSVs for clarity)
    sd_writeHeader("/bmp280.csv", "Temperature_C,Pressure_hPa,Altitude_m");
    sd_writeHeader("/dht22.csv", "Temperature_C,Humidity_%");
}

void loop() {
    // BMP280 data
    float t_bmp = bmp280_getTemperature();
    float p_bmp = bmp280_getPressure();
    float alt_bmp = bmp280_getAltitude();

    Serial.printf("[BMP280] T=%.2f°C, P=%.2f hPa, Alt=%.2f m\n", t_bmp, p_bmp, alt_bmp);

    String bmpData = String(t_bmp, 2) + "," + String(p_bmp, 2) + "," + String(alt_bmp, 2);
    sd_append("/bmp280.csv", bmpData);

    // DHT22 data
    float t_dht = dht22_getTemperature();
    float h_dht = dht22_getHumidity();

    if (isnan(t_dht) || isnan(h_dht)) {
        Serial.println("[DHT22] Failed to read sensor!");
    } else {
        Serial.printf("[DHT22] T=%.2f°C, H=%.2f%%\n", t_dht, h_dht);

        String dhtData = String(t_dht, 2) + "," + String(h_dht, 2);
        sd_append("/dht22.csv", dhtData);
    }

    delay(2000);
}
