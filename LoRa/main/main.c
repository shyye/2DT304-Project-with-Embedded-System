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
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h" // TODO: Check if this is needed
#include "esp_timer.h"

#include "esp_now.h"
#include <string.h>
#include "esp_log.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "esp_system.h"

#include "driver/uart.h"

// For button
#include "driver/gpio.h"

// Added for queue functionality
#include "freertos/queue.h"

// LORA TEST TODO:
#include "ttn.h"

// Pins and States
#define BUTTON_PIN 25
#define BUTTON_PRESSED 1
#define BUTTON_NOT_PRESSED 0

// MAC Address of responder - edit as required
uint8_t broadcastAddress[] = {0xa0, 0x76, 0x4e, 0x40, 0x37, 0xe0};
uint8_t broadcastAddress1[] = {0xA0, 0x76, 0x4E, 0x45, 0x53, 0xAC};

// Define a data structure for recieved messages
typedef struct struct_message_received
{
    char a[32];
    int id;
    int buttonValue;
    int hopcount;
} struct_message_received ;

struct_message_received data;


// Define a data structure for checking own button (On the LoRa device )
typedef struct struct_message_lora_button
{
    char a[32];
    int id;
    int buttonValue;
} struct_message_lora_button ;

struct_message_lora_button lora_button;

// Peer info
esp_now_peer_info_t peerInfo;

// Queue handle
QueueHandle_t dataQueue;

// Notify task handle
TaskHandle_t vTaskReceiveDataHandle;


// *** SET ID HERE ***
// *
int id = 00;
// *
// *******************






// LORA TEST TODO:
// Source: https://github.com/manuelbl/ttn-esp32/wiki/Get-Started

// #include "ttn.h"

// NOTE:
// The LoRaWAN frequency and the radio chip must be configured by running 'idf.py menuconfig'.
// Go to Components / The Things Network, select the appropriate values and save.

// Copy the below hex strings from the TTN console (Applications > Your application > End devices
// > Your device > Activation information)

// Source: https://github.com/manuelbl/ttn-esp32/wiki/Get-Started
// AppEUI (sometimes called JoinEUI)
const char *appEui = "0000000000000000";
// DevEUI
const char *devEui = "70B3D57ED00658F3";
// AppKey
const char *appKey = "D676E7D701E4F46C65CA6AD453362466";

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
static uint8_t msgData[] = "Hello, world";


void sendMessages(uint8_t* msgData, size_t len)
{
    printf("Sending message...\n");
    ttn_response_code_t res = ttn_transmit_message(msgData, sizeof(msgData) - 1, 1, false);
    printf(res == TTN_SUCCESSFUL_TRANSMISSION ? "Message sent.\n" : "Transmission failed.\n");
}

void messageReceived(const uint8_t* message, size_t length, ttn_port_t port)
{
    printf("Message of %d bytes received on port %d:", length, port);
    for (int i = 0; i < length; i++)
        printf(" %02x", message[i]);
    printf("\n");
}
// LORA TEST END TODO:










// TODO: Should all functions have prototypes here?
// ANd should no argument functions have void in their parameter?
void sendDataToQueue(void *data); 

// Source: https://github.com/espressif/esp-now/blob/master/examples/get-started/main/app_main.c
// TODO: Check provisioner vs. responder: https://github.com/espressif/esp-now/blob/master/examples/provisioning/main/app_main.c
// TODO: Byt namn till wifi_init ?
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

// Alternative from Arduino:
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
    printf("\r\nLast Packet Send Status:\t");
    printf(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

// Callback function executed when data is received
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len)
{
    // Copy the incoming data to the data structure
    memcpy(&data, incomingData, sizeof(data));
    // printf("Bytes received: %d\n", len);

    // Notify recieve task
    xTaskNotifyGive(vTaskReceiveDataHandle); // TODO: Check if this is correct (vTaskReceiveDataHandle

}

// TODO: Change to vTaskFunction
void sendData()
{
    data.id = id;
    data.hopcount = data.hopcount - 1;

    if (data.buttonValue == BUTTON_PRESSED)
    {
        strcpy(data.a, "Lifebuoy is here");     
    }
    else
    {
        strcpy(data.a, "Lifebuoy is gone!");
    }

    // Send message via ESP-NOW
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&data, sizeof(data));
    esp_err_t result1 = esp_now_send(broadcastAddress1, (uint8_t *)&data, sizeof(data));

    if (result == ESP_OK)
    {
        printf("Sending confirmed");
    }
    else
    {
        printf("Sending error");
    }
}

void sendDataToQueue(void *data)
{
    // Send data to queue
    xQueueSendToBack(dataQueue, &data, 0);
}


/**
 * Read button value from GPIO (LoRa 32)
 * Store in data
 */
void vTaskReadButtonValue(void *pvParameters)
{
    for (;;)
    {
        // Read button from GPIO
        int newButtonValue = gpio_get_level(BUTTON_PIN);

        // Send data if button vanlue has changed
        if (newButtonValue != lora_button.buttonValue)
        {
            printf("\nButton value has changed LoRa\n");
            // Store new value in data.buttonValue
            lora_button.buttonValue = newButtonValue; //TODO: ID always same, dooesnät need to update

            // Send data
            printf("TEST fom own button\n");

            lora_button.id = id;

            if (lora_button.buttonValue == BUTTON_PRESSED)
            {
                strcpy(lora_button.a, "Lifebuoy is here");     
            }
            else
            {
                strcpy(lora_button.a, "Lifebuoy is gone!");
            }

            // Send message to queue
            sendDataToQueue(&lora_button);
        }

        // printf("Task Check Button Value is running: %lld\n", esp_timer_get_time() / 1000);
        // vTaskDelay(1000 / portTICK_PERIOD_MS);
        vTaskDelay(1); // 1 == 10 ms, to avoid watchdog timer to get triggered, TODO: Check if this is a good idea
    }
}

/**
 * Send data to database via LoRa
 */
void vTaskSendDataToDatabase(void *pvParameters)
{
    for (;;)
    {
        // if (xQueueReceive(dataQueue, &data, 0) == pdTRUE)
        // TODO: The above solution triggers watchdog timer
        if (xQueueReceive(dataQueue, &data, portMAX_DELAY))
        {
            printf("---> Data is in queue\n");
            printf("---> Should now send data to database\n");

            // Test message TODO: test this
            uint8_t testMsg[] = {0x01, 0x02, 0x03};
            sendMessages(testMsg, sizeof(testMsg));
        }
        
    }
    vTaskDelete(NULL);
}

/**
 * Callback function when data is received via bluetooth or WiFi from Seeed devices
 * Maybe this should just be an ordinary fumction and not a task
 * TODO:
 */
void vTaskReceiveData(void *pvParameters)
{
    for (;;)
    {
        // Wait for the notification
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        printf("Data received: %s\n", data.a);
        printf("ID of the sender: %d\n", data.id);
        printf("Button value: %d\n", data.buttonValue);
        printf("Hops left: %d\n", data.hopcount);

        // TODO: Check this structure
        printf("---> OnDataRecv, sending to queue\n");
        // Send data to queue
        //sendData();
        sendDataToQueue(&data);
    }
}



void app_main(void)
{    
    // Create the queue
    // TODO: SHould put the que size 10 as a global variable
    dataQueue = xQueueCreate(10, sizeof(struct_message_received)); // TODO: Check if struct_message is standard or can be whatever we created aboive


    // Set up Serial Monitor
    // Serial.begin(115200);
    // TODO: Är detta uart config här: https://github.com/espressif/esp-now/blob/master/examples/get-started/main/app_main.c
    
    
    // Set ESP32 as a Wi-Fi Station
    // WiFi.mode(WIFI_STA);
    app_wifi_init();

    // Initilize ESP-NOW
    if (esp_now_init() != ESP_OK)
    {
        printf("\nError initializing ESP-NOW");
        return;
    }

    // Register callback function
    esp_now_register_recv_cb(OnDataRecv);
    esp_now_register_send_cb(OnDataSent);

    // Register peer
    memcpy(peerInfo.peer_addr, broadcastAddress, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;

    // Add peer
    if (esp_now_add_peer(&peerInfo) != ESP_OK)
    {
        printf("Failed to add peer");
        return;
    }

    memcpy(peerInfo.peer_addr, broadcastAddress1, 6);
    peerInfo.channel = 1;
    peerInfo.encrypt = false;

    // Add peer
    if (esp_now_add_peer(&peerInfo) != ESP_OK)
    {
        printf("Failed to add peer");
        return;
    }






    // LORA TEST TODO:

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
    ttn_on_message(messageReceived);

    // ttn_set_adr_enabled(false);
    // ttn_set_data_rate(TTN_DR_US915_SF7);
    // ttn_set_max_tx_pow(14);

    printf("Joining...\n");
    if (ttn_join())
    {
        printf("Joined.\n");
        // xTaskCreate(sendMessages, "send_messages", 1024 * 4, (void* )0, 3, NULL);

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


        // // Test message
        // uint8_t testMsg[] = {0x01, 0x02, 0x03};
        // sendMessages(testMsg, sizeof(testMsg));

        // Test send message to queue
    }
    else
    {
        printf("Join failed. Goodbye\n");
    }

    // LORA TEST END TODO:










    // Create tasks
    // Parameter #5 is the priority of the task
    xTaskCreate(vTaskReadButtonValue, "Check Button Value", 2048, NULL, 1, NULL); 
    xTaskCreate(vTaskSendDataToDatabase, "Send Data To Database", 2048, NULL, 1, NULL);
    xTaskCreate(vTaskReceiveData, "Receive Data", 2048, NULL, 1, &vTaskReceiveDataHandle); // TODO: Check if this is correct (vTaskReceiveDataHandle)
}