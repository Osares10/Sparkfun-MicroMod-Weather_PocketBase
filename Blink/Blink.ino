// Blink a LED on the MicroMod Weather (ESP32) board

int ledPin = 2; // LED is connected to GPIO2

void setup() {
  pinMode(ledPin, OUTPUT); // Set GPIO2 to output mode
  Serial.begin(115200); // Initialize serial port
}

void loop() {
  digitalWrite(ledPin, HIGH); // Turn LED on
  delay(1000); // Wait for 1000 millisecond(s)
  Serial.println("The LED is on."); // Print a message 
  digitalWrite(ledPin, LOW); // Turn LED off
  delay(1000); // Wait for 1000 millisecond(s)
}