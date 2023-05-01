# Sparkfun-MicroMod-Weather_PocketBase
Integration of the Sparkfun MicroMod Weather Station with PocketBase with added SD backup

Project developed in arduino, the documentation will not be very extensive, but it will be enough to understand the operation of the code, if you have basic knowledge of the technologies used it should be very easy to understand.

## Hardware
- [SparkFun MicroMod Weather Carrier Board](https://www.sparkfun.com/products/16794)
- [SparkFun MicroMod ESP32 Processor](https://www.sparkfun.com/products/16781)
- [Weather Meter Kit](https://www.sparkfun.com/products/15901)
- microSD (must be formatted in FAT32)

## Libraries
- [Adafruit BME280](https://github.com/adafruit/Adafruit_BME280_Library)
- [Adafruit VEML6075](https://github.com/adafruit/Adafruit_VEML6075)


The following libraries can be installed from the Arduino IDE Library Manager:
- WiFi
- sys/time
- HTTPClient
- esp_sleep

## References
- [Weather Meter Hookup Guide](https://learn.sparkfun.com/tutorials/weather-meter-hookup-guide)
- [MicroMod ESP32 Processor Board Hookup Guide](https://learn.sparkfun.com/tutorials/micromod-esp32-processor-board-hookup-guide)
- [MicroMod Weather Carrier Board Hookup Guide](https://learn.sparkfun.com/tutorials/micromod-weather-carrier-board-hookup-guide)

## Credentials and keys

Remember to change the constants in the `Station_PocketBase.ino` and `Station_PocketBase_SD.ino` files with your credentials and keys.

Wifi credentials:
```ino
const char* ssid = "SSID"; // Network SSID
const char* password = "password"; // Network password
```

PocketBase credentials:
```ino
http.begin("https://custom.domain/api/collections/collection_name/records"); // Specify the URL
```

**Note:** Integration of the AS3935 Lightning Detector is not yet implemented.