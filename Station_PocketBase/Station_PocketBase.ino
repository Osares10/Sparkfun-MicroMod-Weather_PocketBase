// Station code for the Sparkfun MicroMod Weather (ESP32) project
// It's missing AS3935 integration, but it's a good start
// It should print out the weather data to the serial monitor each duration minutes
// It should also send the weather data to a PocketBase server every 5 minutes

#include <Wire.h> 
#include "SparkFunBME280.h"
#include <WiFi.h>
#include <sys/time.h>
#include <HTTPClient.h>
#include <SparkFun_VEML6075_Arduino_Library.h>

BME280 bme280; // Instance of the BME280 class
VEML6075 veml6075; // Create a VEML6075 object

int windDirectionSensor = A1; // Analog pin for wind direction
int windSpeedSensor = D0; // Digital I/O pin for wind speed
int rainSensor = D1; // Digital I/O pin for rain fall

const int duration = 4; // Duration of the report in minutes

int remainingMinutes, remainingSeconds; // Variables to store the remaining minutes and seconds

String currentTime; // Variable to store the current time
String minuteString; // Variable to store the minutes from the time
int minuteInt; // Variable to store the minutes as an int

volatile int windSpeedCount = 0; // Variable to store the number of wind pulses
volatile int rainCount = 0; // Variable to store the number of rain tips

float xSum = 0, ySum = 0; // Variables to store the sum of the x and y values
float x, y, theta, averageWindDirection, averageWindSpeed; // Variables to store the x, y, theta, and average wind direction and speed

struct WeatherData { // Struct to store the weather data
    String time;
    float temperature;
    float humidity;
    float dewpoint;
    float pressure;
    float rainFall;
    float windSpeed;
    float windDirection;
    float uva;
    float uvb;
    float uvindex;
};
WeatherData weather;

struct WindData { // Struct to store the wind data
    float speed;
    float direction;
    boolean reading;
};
WindData wind[duration * 6];

const char* ssid = "SSID"; // Network SSID
const char* password = "password"; // Network password

const char* ntpServer = "pool.ntp.org"; // NTP server
const long gmtOffset_sec = 0; // Offset from GMT
const int daylightOffset_sec = 0; // Offset from daylight savings time

void setup() {
    Serial.begin(115200); // Start the serial monitor
    while (!Serial); // Wait for user to open serial monitor

    connectToWifi(); // Connect to the Wi-Fi network

    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    while (!time(nullptr)) {
        delay(1000);
        Serial.println("Waiting for time sync...");
    }
    Serial.println("Time synced");

    Wire.begin(); // Join the I2C bus

    if (bme280.begin() == false) { // Connect to the BME280
        Serial.println("BME280 did not respond."); // Print an error message if the BME280 does not respond
        while(1); // Freeze
    }

    if (veml6075.begin() == false) {
        Serial.println("VEML6075 did not respond.");
        while(1); // Freeze
    }

    pinMode(LED_BUILTIN, OUTPUT); // Set the LED pin as an output
    pinMode(windSpeedSensor, INPUT_PULLUP); // Set the wind speed pin as an input
    pinMode(rainSensor, INPUT_PULLUP); // Set the rain pin as an input
    attachInterrupt(digitalPinToInterrupt(windSpeedSensor), windSpeedIRQ, FALLING); // Attach the wind speed interrupt
    attachInterrupt(digitalPinToInterrupt(rainSensor), rainIRQ, FALLING); // Attach the rain interrupt
    interrupts(); // Enable interrupts

    Serial.println("Setup complete"); // Print a message to the serial monitor
    Serial.println("Waiting for minute ending in 1 or 6 to start report"); // Print a message to the serial monitor
}

void loop() {
    digitalWrite(LED_BUILTIN, HIGH); // Turn on the LED

    minuteInt = getMinute(); // Get the current minute

    if (minuteInt % 5 == 1) {
        Serial.println("Starting report..."); // Print a message to the serial monitor
        rainCount = 0; // Reset the rain count
        for (int i = 0; i < (duration * 6); i++) { // Reset the wind direction array
                wind[i].reading = false;
                wind[i].direction = 0;
                wind[i].speed = 0;
        }
        for (int i = 0; i < (duration * 6); i++) { // Loop for the duration of the report, taking measurements every 10 seconds
            Serial.print("Taking wind measurements, this process will be finished in: "); // Print a message to the serial monitor to let the user know the program is working
            remainingMinutes = duration - ((i / 6) + !!(i % 6));
            remainingSeconds = 60 - ((i % 6) * 10);
            Serial.print(remainingMinutes);
            Serial.print(":");
            if (remainingSeconds == 60) {
                Serial.println("00");
            } else {
                Serial.println(remainingSeconds);
            }

            windSpeedCount = 0; // Reset the wind speed count
            delay(10 * 1000); // Wait for a minute
            wind[i].speed = (windSpeedCount * 1.2959) / (10); // Calculate the wind speed
            wind[i].direction = getWindDirection(); // Calculate the wind direction
            wind[i].reading = true; // Set the reading flag to true
        }
        weather.rainFall = (rainCount * 0.011 * 60) / (duration); // Calculate the rain fall in inches/hour
        getAverageWind(averageWindDirection, averageWindSpeed); // Get the average wind direction and speed
        weather.windDirection = averageWindDirection; // Set the average wind direction to the weather data
        weather.windSpeed = averageWindSpeed; // Set the average wind speed to the weather data
        weather.temperature = bme280.readTempC(); // Get the temperature in degrees Celsius
        weather.humidity = bme280.readFloatHumidity(); // Get the humidity in percent
        weather.dewpoint = bme280.dewPointC(); // Get the dew point in degrees Celsius
        weather.pressure = bme280.readFloatPressure() / (20 * 133.32239); // Convert the pressure from Pascals to inches of mercury

        weather.uva = veml6075.uva(); // Get the UVA value
        weather.uvb = veml6075.uvb(); // Get the current time in UTC
        weather.uvindex = veml6075.index(); // Get the UV index

        weather.time = getUTCTime(); // Get the current time in UTC
        printWeather(); // Print the weather data to the serial monitor
        sendWeatherDataToPocketBase(); // Send the weather data to PocketBase
    }
    delay(1000); // Wait for a second
    digitalWrite(LED_BUILTIN, LOW); // Turn off the LED
}

String getUTCTime() {
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    time_t now = tv.tv_sec;
    struct tm timeinfo;
    gmtime_r(&now, &timeinfo);
    char buffer[30];
    snprintf(buffer, sizeof(buffer), "%04d-%02d-%02d %02d:%02d:%02d.%03ldZ",
        timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
        timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec,
        tv.tv_usec / 1000);
    return String(buffer);
}

int getMinute() { // Get the current minute
    currentTime = getUTCTime();
    minuteString = currentTime.substring(14, 16);
    minuteInt = minuteString.toInt();
    return minuteInt;
}

void connectToWifi() {
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password); // Connect to the network

    while (WiFi.status() != WL_CONNECTED) { // Wait for the Wi-Fi to connect
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP()); // Print the local IP address
}

void printWeather() {
    Serial.print("Time: ");
    Serial.print(weather.time); // Time in UTC
    Serial.print("Z   Temperature: ");
    Serial.print(weather.temperature, 2); // Temperature in degrees Celsius
    Serial.print("   Humidity: ");
    Serial.print(weather.humidity, 2); // Humidity in percent
    Serial.print("   Dewpoint: ");
    Serial.print(weather.dewpoint, 2); // Dew point in degrees Celsius
    Serial.print("   Pressure: ");
    Serial.print(weather.pressure, 2); // Pressure in inches of mercury
    Serial.print("   Wind Speed: ");
    Serial.print(weather.windSpeed, 2); // Wind speed in knots
    Serial.print("   Wind Direction: ");
    Serial.print(weather.windDirection, 2); // Average wind direction in degrees
    Serial.print("   Rain Fall: ");
    Serial.print(weather.rainFall, 2); // Rain fall in inches/hour
    Serial.print("   UVA: ");
    Serial.print(weather.uva); // UVA value
    Serial.print("   UVB: ");
    Serial.print(weather.uvb); // UVB value
    Serial.print("   UV Index: ");
    Serial.println(weather.uvindex); // UV index
}

int getWindDirection() {
    unsigned int reading;
    reading = analogRead(windDirectionSensor); // Read the analog value from the wind direction sensor

    if (  10 < reading && reading <=  150) return (288);
    if ( 150 < reading && reading <=  250) return (264);
    if ( 250 < reading && reading <=  400) return (336);
    if ( 400 < reading && reading <=  600) return (312);
    if ( 650 < reading && reading <=  850) return ( 24);
    if ( 850 < reading && reading <= 1050) return (  0);
    if (1300 < reading && reading <= 1500) return (216);
    if (1500 < reading && reading <= 1700) return (240);
    if (2000 < reading && reading <= 2200) return ( 72);
    if (2200 < reading && reading <= 2400) return ( 48);
    if (2400 < reading && reading <= 2600) return (168);
    if (2800 < reading && reading <= 3000) return (192);
    if (3000 < reading && reading <= 3200) return (120);
    if (3300 < reading && reading <= 3500) return (144);
    if (3700 < reading && reading <= 3900) return ( 96);
    return (-1);
}

void getAverageWind(float& averageWindDirection, float& averageWindSpeed) {
    for (int i = 0; i < (duration * 6); i++) {
        theta = radians(wind[i].direction); // convert angle to radians
        x = wind[i].speed * cos(theta);
        y = wind[i].speed * sin(theta);
        xSum += x;
        ySum += y;
    }
    averageWindDirection = degrees(atan2(ySum / (duration * 6), xSum / (duration * 6))); // convert radians to degrees
    if (averageWindDirection < 0) averageWindDirection += 360; // convert negative angles to positive
    averageWindSpeed = sqrt(pow(xSum / (duration * 6), 2) + pow(ySum / (duration * 6), 2)); // calculate average speed

    xSum = 0;
    ySum = 0;
}

void rainIRQ()
{
    rainCount++;
    // Serial.println("Rain clicked");
}

// Function is called when the magnet in the anemometer is activated
void windSpeedIRQ()
{
    windSpeedCount++;
    // Serial.println("Wind clicked");
}

void sendWeatherDataToPocketBase() {
    Serial.println("Sending weather data to PocketBase...");

    HTTPClient http;

    // Set the PocketBase endpoint URL
    http.begin("https://custom.domain/api/collections/collection_name/records");

    // Set the HTTP headers
    http.addHeader("Content-Type", "application/json");

    // Create the JSON payload
    String payload = "{\"time\":\"" + weather.time +
                    "\",\"temperature\":" + String(weather.temperature) +
                    ",\"humidity\":" + String(weather.humidity) +
                    ",\"dewpoint\":" + String(weather.dewpoint) +
                    ",\"pressure\":" + String(weather.pressure) +
                    ",\"rainFall\":" + String(weather.rainFall) +
                    ",\"windSpeed\":" + String(weather.windSpeed) +
                    ",\"windDirection\":" + String(weather.windDirection) +
                    ",\"uva\":" + String(weather.uva) +
                    ",\"uvb\":" + String(weather.uvb) + 
                    ",\"uvindex\":" + String(weather.uvindex) + "}";

    // Send the POST request with the payload
    int httpCode = http.POST(payload);

    // Check if the request was successful
    if(httpCode == 200) {
        Serial.println("Data sent to PocketBase successfully");
    } else {
        Serial.println("Error sending data to PocketBase");
        Serial.print("HTTP code: ");
        Serial.println(httpCode);
    }

    // Free resources
    http.end();
}