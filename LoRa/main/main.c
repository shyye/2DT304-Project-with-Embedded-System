/*
 * REFERENCES:
 *
 * FreeRTOS
 * Two tasks - FreeRTOS ESP IDF for esp32 by SIMS IOT DEVICES
 * https://www.youtube.com/watch?v=bEhZp4Adghc
 *
 *
 * ESP-NOW
 * ESP NOW - Peer to Peer ESP32 Communications
 * https://dronebotworkshop.com/esp-now/
 *
 *
 * Arduino Vs. ESP-IDF - Setup(), Loop(), app_main()
 * ESP-IDF for Arduino Users Tutorials Part 3, App Setup and Loop
 * https://aliafshar.medium.com/esp-idf-for-arduino-users-tutorials-part-3-app-setup-and-loop-9086726627
 *
 *
 * TTN
 * https://github.com/manuelbl/ttn-esp32 TODO: CHeck how to write this reference
 * https://github.com/manuelbl/ttn-esp32/wiki/Get-Started
 * *******************************************************************************
 * 
 * ttn-esp32 - The Things Network device library for ESP-IDF / SX127x
 * 
 * Copyright (c) 2021 Manuel Bleichenbacher
 * 
 * Licensed under MIT License
 * https://opensource.org/licenses/MIT
 *
 * Sample program for C showing how to send and receive messages.
 ******************************************************************************/

#include <stdio.h>
#include <string.h>

// WiFi
#include "esp_wifi.h"
#include "nvs_flash.h"

// ESP-NOW
#include "esp_now.h"

// LoRa & TTN
#include "ttn.h"
#include "config.h"

// FreeRTOS
#include "freertos/FreeRTOS.h"

// Button
#include "driver/gpio.h"


// --- ESP-NOW CONSTANTS ---
// TODO: Don't need broadcast, will not send data to the devices
// MAC Address of responder - edit as required
// uint8_t broadcastAddress[] = {0xa0, 0x76, 0x4e, 0x40, 0x37, 0xe0};
// uint8_t broadcastAddress1[] = {0xA0, 0x76, 0x4E, 0x45, 0x53, 0xAC}; // TODO:

// Data structure for recieved messages
typedef struct struct_message_received
{
    char a[32]; // TODO:
    int id; // TODO: define as byte?
    int buttonValue;
    int hopcount;
} struct_message_received;
struct_message_received data; // TODO:

// Peer info
esp_now_peer_info_t peerInfo;


// --- LoRa & TTN ---
// AppEUI (sometimes called JoinEUI)
const char *appEui = APP_EUI;
// DevEUI
const char *devEui = DEV_EUI;
// AppKey
const char *appKey = APP_KEY;

// Pins and other resources
#define TTN_SPI_HOST      HSPI_HOST
#define TTN_SPI_DMA_CHAN  1
#define TTN_PIN_SPI_SCLK  5
#define TTN_PIN_SPI_MOSI  27
#define TTN_PIN_SPI_MISO  19
#define TTN_PIN_NSS       18
#define TTN_PIN_RXTX      TTN_NOT_CONNECTED
#define TTN_PIN_RST       14
#define TTN_PIN_DIO0      26
#define TTN_PIN_DIO1      33

#define TX_INTERVAL 30     // Interval between transmissions (seconds) TODO: Maybe not used now
// static uint8_t msgData[] = "Hello, world"; //TODO: change
static uint8_t ttnData[3];      // 3 Bytes to TTN


// --- FreeRTOS CONSTANTS ---
TaskHandle_t vTaskReceiveDataHandle;        // Notify task handle

// Queue handle
QueueHandle_t dataQueue;

// Pins and States
#define BUTTON_PIN 25
#define BUTTON_PRESSED 1
#define BUTTON_NOT_PRESSED 0


// --- WIFI METHODS ---
// Source: https://github.com/espressif/esp-now/blob/master/examples/get-started/main/app_main.c
// TODO: Check provisioner vs. responder: https://github.com/espressif/esp-now/blob/master/examples/provisioning/main/app_main.c
static void app_wifi_init()
{
    // TODO: Here or in app_main?
    nvs_flash_init();   // Not in the example above but necessary to prevent errors

    esp_event_loop_create_default();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));
    ESP_ERROR_CHECK(esp_wifi_start());
}


// --- ESP-NOW METHODS ---
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)      // TODO: not yet used
{
    printf("\r\nLast Packet Send Status:\t");
    printf(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

// Callback function executed when data is received
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len)
{
    // Copy the incoming data to the data structure
    memcpy(&data, incomingData, sizeof(data));
    printf("Bytes received: %d\n", len);

    // Print the received data
    printf("Received Data:\n");
    printf("a: %s\n", data.a);
    printf("id: %d\n", data.id);
    printf("buttonValue: %d\n", data.buttonValue);
    printf("hopcount: %d\n", data.hopcount);
    printf("-------\n");
    
    // Notify receive task
    xTaskNotifyGive(vTaskReceiveDataHandle); // TODO: Check if this is correct (vTaskReceiveDataHandle)
}

static void app_esp_now_init()
{
    if (esp_now_init() != ESP_OK)
    {
        printf("\nError initializing ESP-NOW");
        return;
    }

    // Register callback function
    esp_now_register_recv_cb(OnDataRecv);

    // TODO: Could implement send feature if the LoRa should be able to send data to the Seeed devices

    // TODO: Don't need, will not send data to the devices
    // esp_now_register_send_cb(OnDataSent);

    // // Register peer
    // memcpy(peerInfo.peer_addr, broadcastAddress, 6);
    // peerInfo.channel = 0;
    // peerInfo.encrypt = false;

    // // Add peer
    // if (esp_now_add_peer(&peerInfo) != ESP_OK)
    // {
    //     printf("Failed to add peer");
    //     return;
    // }

    // memcpy(peerInfo.peer_addr, broadcastAddress1, 6);
    // peerInfo.channel = 1;
    // peerInfo.encrypt = false;

    // // Add peer
    // if (esp_now_add_peer(&peerInfo) != ESP_OK)
    // {
    //     printf("Failed to add peer");
    //     return;
    // }
}


// --- LoRa & TTN METHODS ---
void TTN_sendMessage(uint8_t* ttnData, size_t len)
{
    printf("Sending message...\n");
    ttn_response_code_t res = ttn_transmit_message(ttnData, sizeof(ttnData) - 1, 1, false);
    printf(res == TTN_SUCCESSFUL_TRANSMISSION ? "Message sent.\n" : "Transmission failed.\n");
}

void TTN_messageReceived(const uint8_t* message, size_t length, ttn_port_t port)
{
    printf("Message of %d bytes received on port %d:", length, port);
    for (int i = 0; i < length; i++)
        printf(" %02x", message[i]);
    printf("\n");
}

static void app_lora_ttn_init()
{
    esp_err_t err;
    // Initialize the GPIO ISR handler service
    err = gpio_install_isr_service(ESP_INTR_FLAG_IRAM);
    ESP_ERROR_CHECK(err);
    
    // Initialize the NVS (non-volatile storage) for saving and restoring the keys
    err = nvs_flash_init();
    ESP_ERROR_CHECK(err);

    // Initialize SPI bus
    spi_bus_config_t spi_bus_config = {
        .miso_io_num = TTN_PIN_SPI_MISO,
        .mosi_io_num = TTN_PIN_SPI_MOSI,
        .sclk_io_num = TTN_PIN_SPI_SCLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1
    }; 
    err = spi_bus_initialize(TTN_SPI_HOST, &spi_bus_config, TTN_SPI_DMA_CHAN);
    ESP_ERROR_CHECK(err);

    // Initialize TTN
    ttn_init();

    // Configure the SX127x pins
    ttn_configure_pins(TTN_SPI_HOST, TTN_PIN_NSS, TTN_PIN_RXTX, TTN_PIN_RST, TTN_PIN_DIO0, TTN_PIN_DIO1);

    // The below line can be commented after the first run as the data is saved in NVS
    ttn_provision(devEui, appEui, appKey);

    // Register callback for received messages
    ttn_on_message(TTN_messageReceived);

    // ttn_set_adr_enabled(false);
    // ttn_set_data_rate(TTN_DR_US915_SF7);
    // ttn_set_max_tx_pow(14);

    printf("Joining...\n");
    if (ttn_join())
    {
        printf("Joined.\n");
    }
    else
    {
        printf("Join failed. Goodbye\n");
    }
}


// --- FreeRTOS METHODS ---
/**
 * Callback function when data is received via bluetooth or WiFi from Seeed devices
 */
void sendDataToQueue(void *data)
{
    // Send data to queue
    // xQueueSendToBack(dataQueue, &data, 0);
    xQueueSendToBack(dataQueue, data, 0);   //TODO: check value 0 wait when queu is full

    printf("Data sent to queue\n");         //TODO: remove
}

void vTaskReceiveData(void *pvParameters)
{
    while(1)
    {   
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);        // Wait for notification from onDataRecv()

        // TODO: Check this structure
        printf("---> OnDataRecv, sending to queue\n");

        // Test
        // ttnData[0] = 10; // ID value
        // ttnData[1] = 01; // Button value
        // ttnData[2] = 03; // Hopcount

        ttnData[0] = 10;                // Zone value for this LoRa device
        ttnData[1] = data.id;           // Device ID, incoming data from Seeed device
        ttnData[2] = data.buttonValue;  // Button value, , incoming data from Seeed device     
        sendDataToQueue(ttnData);
    }
}

/**
 * Send data to database via LoRa
 */
void vTaskSendDataToDatabase(void *pvParameters)
{
    while(1)
    {
        // if (xQueueReceive(dataQueue, &data, 0) == pdTRUE)
        // TODO: The above solution triggers watchdog timer
        // if (xQueueReceive(dataQueue, &data, portMAX_DELAY))
        if (xQueueReceive(dataQueue, &ttnData, portMAX_DELAY))
        {
            printf("---> Data is in queue\n");
            printf("---> Should now send data to database\n");

            TTN_sendMessage(ttnData, sizeof(ttnData));            
        }
        
    }
    vTaskDelete(NULL);
}




// --- MAIN ---
void app_main()
{
   // TODO: Create Queue

   // Wifi
   app_wifi_init();

   // ESP-NOW
   app_esp_now_init();

   // LoRA & TTN
   app_lora_ttn_init();

   // FreeRTOS
   dataQueue = xQueueCreate(10, sizeof(ttnData)); // TODO: Check if struct_message is standard or can be whatever we created aboive, ALso make constant for 10 queue size
   xTaskCreate(vTaskReceiveData, "Receive Data", 2048, NULL, 1, &vTaskReceiveDataHandle); // TODO: Check if this is correct (vTaskReceiveDataHandle)
   xTaskCreate(vTaskSendDataToDatabase, "Send Data To Database", 2048, NULL, 1, NULL);
};






// MESSAGE CODES
        // Message type:
        //    00 - Connection Check
        //    01 - Lifebuoy Monitoring

        // Message type, Zone, ID, Button value. (E.g. 01, 03, 01)
        // Message type: 01
        // Zone: 01 - 99
        // ID: 01 - 99
        // Button value:
        //    00 - Missing
        //    01 - In place

        // Message type, Connection, Battery level
        // Message type: 00
        // Conncetion:
        //    00 - No connection
        //    01 - Connection
        // Battery level: 00 - 99
