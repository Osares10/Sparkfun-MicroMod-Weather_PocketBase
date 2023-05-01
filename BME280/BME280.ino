#include <Wire.h>
#include "SparkFunBME280.h"

BME280 bme280Sensor; // Create BME280 object

float RealFloatPressure;

void setup() {
    Serial.begin(115200); // Initialize serial port
    while (!Serial);  // Wait for user to open serial monitor

    Serial.println("MicroMod Weather Carrier Board - BME280 Example");
    Serial.println();

    Wire.begin(); // Join I2C bus

    bme280Sensor.setReferencePressure(101500); // Set sea level pressure to 101325 Pa (default)

    if (bme280Sensor.begin() == false) { // Connect to BME280
        Serial.println("BME280 did not respond.");
        while(1); // Freeze
    }

    pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.print("Temperature: ");
  Serial.println(bme280Sensor.readTempC(), 2);
  Serial.print("Humidity: ");
  Serial.println(bme280Sensor.readFloatHumidity(), 0);
  Serial.print("Pressure: ");

  RealFloatPressure = bme280Sensor.readFloatPressure() / (20 * 133.32239);
  Serial.println(RealFloatPressure, 2);

  Serial.print("Altitude: ");
  Serial.println(bme280Sensor.readFloatAltitudeMeters(), 1);
  Serial.print("Dewpoint: ");
  Serial.println(bme280Sensor.dewPointC(), 2);

  digitalWrite(LED_BUILTIN, LOW);
  delay(1000);
}
