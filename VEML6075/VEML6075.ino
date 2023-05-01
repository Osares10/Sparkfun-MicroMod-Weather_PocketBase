#include <SparkFun_VEML6075_Arduino_Library.h>

VEML6075 veml6075; // Create a VEML6075 object

void setup() {
  Serial.begin(115200);
  while(!Serial); // Wait for user to open serial monitor
  
  Serial.println("MicroMod Weather Carrier Board - VEML6075 Example");

  Wire.begin(); // Join I2C bus

  if (veml6075.begin() == false) {
    Serial.println("VEML6075 did not respond."); // If the sensor does not respond, print an error message
    while(1); // Freeze
  }

  pinMode(LED_BUILTIN, OUTPUT);
  Serial.println("UVA, UVB, UV Index"); // Print the header for the data
  Serial.println();

}

void loop() {
    digitalWrite(LED_BUILTIN, HIGH);
    // Print the UVA, UVB, and UV Index values
    Serial.println("UVA: " + String(veml6075.uva())); 
    Serial.println("UVB: " + String(veml6075.uvb()));
    Serial.println("UV Index: " + String(veml6075.index()));

    digitalWrite(LED_BUILTIN, LOW);
    delay(1000); // Wait 1 second
}
