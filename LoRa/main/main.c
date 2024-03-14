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
#include "esp_event.h"

// GPIO
#include "driver/gpio.h"
#include "nvs_flash.h"


// LoRa device - CONSTANTS
uint8_t zone = 15;


// ESP-NOW -CONSTANTS
typedef struct struct_message_received
{
    char a[32]; // TODO:
    int id; // TODO: define as byte?
    int buttonValue;
    int hopcount;
} struct_message_received;

struct_message_received seeedData;



// LoRa & TTN - CONSTANTS
// NOTE: TODO:
// The LoRaWAN frequency and the radio chip must be configured by running 'idf.py menuconfig'.
// Go to Components / The Things Network, select the appropriate values and save.

// Copy the below hex strings from the TTN console (Applications > Your application > End devices
// > Your device > Activation information)

// AppEUI (sometimes called JoinEUI)
const char *appEui = APP_EUI;
// DevEUI
const char *devEui = DEV_EUI;
// AppKey
const char *appKey = APP_KEY;

// Pins and other resources
#define TTN_SPI_HOST      SPI2_HOST
#define TTN_SPI_DMA_CHAN  SPI_DMA_DISABLED
#define TTN_PIN_SPI_SCLK  5
#define TTN_PIN_SPI_MOSI  27
#define TTN_PIN_SPI_MISO  19
#define TTN_PIN_NSS       18
#define TTN_PIN_RXTX      TTN_NOT_CONNECTED
#define TTN_PIN_RST       14
#define TTN_PIN_DIO0      26
#define TTN_PIN_DIO1      35

#define TX_INTERVAL 30 // TODO:
static uint8_t msgData[3];      // 3 Bytes to TTN


// FreeRTOS - CONSTANTS
QueueHandle_t ttnQueue;


// ESP-NOW - FUNCTIONS
// Callback function executed when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
    printf("\r\nLast Packet Send Status:\t");
    printf(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

// Callback function executed when data is received TODO: SHould this create an RTOS task?
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len)
{
    // Copy the incoming data to the data structure
    memcpy(&seeedData, incomingData, sizeof(seeedData));

    uint8_t ttnData[3];
    ttnData[0] = zone;
    ttnData[1] = seeedData.id;
    ttnData[2] = seeedData.buttonValue;

    // uint8_t test[3] = {10, 02, 04};
    // xQueueSend(ttnQueue, test, portMAX_DELAY);

    xQueueSend(ttnQueue, ttnData, portMAX_DELAY);
}

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

static void app_esp_now_init()
{
    if (esp_now_init() != ESP_OK)
    {
        printf("\nError initializing ESP-NOW");
        return;
    }

    // Register callback function
    esp_now_register_recv_cb(OnDataRecv);
}


// LoRa & TTN - FUNCTIONS
void sendMessages(void* pvParameter)        // TODO: Not used
{
    uint8_t* ttnData = (uint8_t*) pvParameter;
    printf("Sending message...\n");
    ttn_response_code_t res = ttn_transmit_message(ttnData, sizeof(ttnData), 1, false);
    printf(res == TTN_SUCCESSFUL_TRANSMISSION ? "Message sent.\n" : "Transmission failed.\n");

    vTaskDelete(NULL);
}

// TODO: Not currently used
void messageReceived(const uint8_t* message, size_t length, ttn_port_t port)
{
    printf("Message of %d bytes received on port %d:", length, port);
    for (int i = 0; i < length; i++)
        printf(" %02x", message[i]);
    printf("\n");
}

static void app_lora_ttn_init() {
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
    }
    else
    {
        printf("Join failed. Goodbye\n");
    }
}






void vCheckQueueTask(void* pvParameter)
{
    uint8_t data[3];   // Variable to store the data in the queue TODO: Fix variable size
    BaseType_t xStatus; 
    while (1) {

        xStatus = xQueueReceive( ttnQueue, &data, portMAX_DELAY );
        if( xStatus == pdPASS )
        {
            /* Data was successfully received from the queue, print out the
            received value. */
            printf("Received = ");
            for (int i = 0; i < sizeof(data); i++) {
                printf("%d ", data[i]);
            }
            printf("\r\n");

            // xTaskCreate(sendMessages, "send_messages", 1024 * 4, (void* )data, 1, NULL); // TODO: check this more

            ttn_response_code_t res = ttn_transmit_message(data, sizeof(data), 1, false);
            printf(res == TTN_SUCCESSFUL_TRANSMISSION ? "Message sent.\n" : "Transmission failed.\n");
        }
        else
        {
            /* Data was not received from the queue even after waiting for
            100ms. This must be an error as the sending tasks are free
            running and will be continuously writing to the queue. */
            printf( "Could not receive from the queue.\r\n" );
        } 
    }
}


void app_main(void)
{
    // Wifi
   app_wifi_init();

   // ESP-NOW
   app_esp_now_init();

   // LoRA & TTN
   app_lora_ttn_init();

    // Queue with data to TTN
    ttnQueue = xQueueCreate(10, sizeof(msgData));
    if (ttnQueue != NULL) {

        printf("ttnQueue succesfully created\n");

        xTaskCreate(vCheckQueueTask, "task_check_queue", 8192, NULL, 2, NULL);      // Check Stack size, 4096 works also

    } else {
        printf("ttnQueue failed to create\n");
    }   
    
    
}