# Setup Hardware

## Get MAC Addresses
For the ESP-NOW/WiFI connection we need the unique MAC address for each device.  

This was done with Arduino IDE.  

For this we used the code below from DroneBot Workshop.  
Source: [DroneBot Workshop - ESP NOW – Peer to Peer ESP32 Communications](https://dronebotworkshop.com/esp-now/)

```
/*
  ESP32 MAC Address printout
  esp32-mac-address.ino
  Prints MAC Address to Serial Monitor
 
  DroneBot Workshop 2022
  https://dronebotworkshop.com
*/
 
// Include WiFi Library
#include "WiFi.h"
 
void setup() {
 
  // Setup Serial Monitor
  Serial.begin(115200);
 
  // Put ESP32 into Station mode
  WiFi.mode(WIFI_MODE_STA);
 
  // Print MAC Address to Serial monitor
  Serial.print("MAC Address: ");
  Serial.println(WiFi.macAddress());
}
 
void loop() {
 
}
```

## Seeed XIAO
### Change settings with VSCode Extension for ESP-IDF
1. Choose ESP32-C3
2. Choose COM PORT
3. Chooose UART

### Troubleshooting
If the device doesn't want to connect.  
Try to press the left button while connecting the USB.

## Heltec
You need to create a file called config.h  
It should be placed in the main folder and contain the following identification and keys from TTN:
```
#define APP_EUI "????????????????"
#define DEV_EUI "?????????????????"
#define APP_KEY "??????????????????????????????????"
```
