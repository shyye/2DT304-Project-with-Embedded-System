
/*
  ESP-NOW Demo - Transmit
  esp-now-demo-xmit.ino
  Sends data to Responder
  
  DroneBot Workshop 2022
  https://dronebotworkshop.com
*/
 
// Include Libraries
#include <esp_now.h>
#include <WiFi.h>
 
// MAC Address of responder - edit as required
uint8_t broadcastAddress[] = {0x78, 0x21, 0x84, 0x9E, 0xFE, 0xCC};
 
// Define a data structure
typedef struct struct_message {
  char a[32];
  int id;
  int buttonValue;
} struct_message;
 
// Create a structured object
struct_message data;
 
// Peer info
esp_now_peer_info_t peerInfo;

// *** SET ID HERE ***
int id = 1;
 
// Callback function called when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}
 
void setup() {
  
  // Set up Serial Monitor
  Serial.begin(115200);
  pinMode(D10, INPUT);
  // Set ESP32 as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
 
  // Initilize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
 
  // Register the send callback
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
}

// 1 = Lifebuoy is here / button is pressed
// 0 = Lifebuoy is gone! / button is not pressed
bool currentState = 1;
bool previousState = 1;
bool sendData = false;
 
void loop() {

  currentState = digitalRead(D10);

  // Send data if button state has changed
  if (currentState != previousState) {
    sendData = true;
  } else {
    sendData = false;
  }

  if (sendData) {

    if (currentState == 1) {
      
      // Format structured data
      strcpy(data.a, "Lifebuoy is here");
      data.id = id;
      data.buttonValue = currentState;

    } else {
      
      // Format structured data
      strcpy(data.a, "Lifebuoy is gone!");
      data.id = id;
      data.buttonValue = currentState;
    }

    
    // Send message via ESP-NOW
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &data, sizeof(data));
    
    if (result == ESP_OK) {
      Serial.println("Sending confirmed");
    }
    else {
      Serial.println("Sending error");
    }

  }

  previousState = currentState;
  delay(2000);
}