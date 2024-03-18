/*
 * REFERENCES:
 *
 * Two tasks - FreeRTOS ESP IDF for esp32 by SIMS IOT DEVICES
 * https://www.youtube.com/watch?v=bEhZp4Adghc
 * 
 * Simple read/write pin example
 * https://esp32tutorials.com/esp32-push-button-esp-idf-digital-input/
 *
 *
 */

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

// Pins and States
#define BUTTON_PIN 10
#define BUTTON_PRESSED 1
#define BUTTON_NOT_PRESSED 0

// *** SET DEVICE ID HERE ***
// *
int id = 1;
// *
// *******************

// MAC Address of responder - edit as required
uint8_t broadcastAddress[] = {0x78, 0x21, 0x84, 0x9E, 0xFE, 0xCC};
uint8_t broadcastAddress1[] = {0xa0, 0x76, 0x4e, 0x40, 0x37, 0xe0};
uint8_t broadcastAddress2[] = {0xA0, 0x76, 0x4E, 0x45, 0x53, 0xAC};
uint8_t broadcastAddress3[] = {0xA0, 0x76, 0x4E, 0x44, 0xC9, 0x78};

// Define a data structure for recieved messages
typedef struct struct_message_received
{
    int id;
    int buttonValue;
    int hopcount;
} struct_message_received;

struct_message_received recdata;

// Define a data structure
typedef struct struct_message
{
    int id;
    int buttonValue;
    int hopcount;
} struct_message;

// Create a structured object
struct_message data;

// Peer info
esp_now_peer_info_t peerInfo;

// Notify task handle
TaskHandle_t vTaskReceiveDataHandle;

static const char* TAG = "MyModule"; 

// Source: https://github.com/espressif/esp-now/blob/master/examples/get-started/main/app_main.c
static void app_wifi_init()
{
    nvs_flash_init(); // Not included in the example from the source above but necessary to prevent errors

    esp_event_loop_create_default();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    uint8_t proto = 0;
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE)); 
    esp_wifi_set_protocol(WIFI_IF_STA, WIFI_PROTOCOL_LR);
    esp_wifi_get_protocol(WIFI_IF_STA, &proto);
    ESP_LOGI(TAG, "%d", proto);
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
    memcpy(&recdata, incomingData, sizeof(recdata));
    // printf("Bytes received: %d\n", len);

    // Notify recieve task
    xTaskNotifyGive(vTaskReceiveDataHandle);
}

void sendData(int id, int hopcount, int buttonValue)
{
    data.id = id;
    data.hopcount = hopcount - 1;

    // Send message via ESP-NOW
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&data, sizeof(data));
    esp_err_t result1 = esp_now_send(broadcastAddress1, (uint8_t *)&data, sizeof(data));
    esp_err_t result2 = esp_now_send(broadcastAddress2, (uint8_t *)&data, sizeof(data));
    esp_err_t result3 = esp_now_send(broadcastAddress3, (uint8_t *)&data, sizeof(data));
    if (result == ESP_OK)
    {
        printf("Sending confirmed");
    }
    else
    {
        printf("Sending error");
    }
}

/**
 * Read button value from GPIO (Seeed)
 * Store in data
 */
void vTaskReadButtonValue(void *pvParameters)
{
    for (;;)
    {
        // Read button from GPIO
        int newButtonValue = gpio_get_level(BUTTON_PIN);

        // Send data if button vanlue has changed
        if (newButtonValue != data.buttonValue)
        {
            printf("\nButton value has changed\n");
            // Store new value in data.buttonValue
            data.buttonValue = newButtonValue;

            // Send data to LoRa device
            sendData(id, 4, data.buttonValue);
        }

        // printf("Task Check Button Value is running: %lld\n", esp_timer_get_time() / 1000);
        // vTaskDelay(1000 / portTICK_PERIOD_MS);
        vTaskDelay(1); // 1 == 10 ms, to avoid watchdog timer to get triggered, TODO: Check if this is a good idea
    }
}

// TODO: Is this used
void vTaskSendData(void *pvParameters)
{
    for (;;)
    {
        // Send data
        printf("Task Send Data is running: %lld\n", esp_timer_get_time() / 1000);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}

void vTaskReceiveData(void *pvParameters)
{
    for (;;)
    {
        // Wait for the notification
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        // printf("\nData received: %s\n", recdata.a);
        printf("ID of the sender: %d\n", recdata.id);
        printf("Button value: %d\n", recdata.buttonValue);
        printf("Hops left: %d\n", recdata.hopcount);

        // TODO: Check this structure
        printf("---> OnDataRecv, sending to queue\n");
        // Send data to queue
        if(recdata.hopcount > 0){
            sendData(recdata.id, recdata.hopcount, recdata.buttonValue);
        }
        // sendDataToQueue(&recdata);
    }
}

void app_main()
{
    // Pin
    gpio_set_direction(BUTTON_PIN, GPIO_MODE_INPUT);

    // WiFi
    app_wifi_init();

    // Initilize ESP-NOW
    if (esp_now_init() != ESP_OK)
    {
        printf("Error initializing ESP-NOW");
        return;
    }

    // Register the send callback
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
    peerInfo.channel = 0;
    peerInfo.encrypt = false;

    // Add peer
    if (esp_now_add_peer(&peerInfo) != ESP_OK)
    {
        printf("Failed to add peer");
        return;
    }

    memcpy(peerInfo.peer_addr, broadcastAddress2, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;

    // Add peer
    if (esp_now_add_peer(&peerInfo) != ESP_OK)
    {
        printf("Failed to add peer");
        return;
    }

    memcpy(peerInfo.peer_addr, broadcastAddress3, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;

    // Add peer
    if (esp_now_add_peer(&peerInfo) != ESP_OK)
    {
        printf("Failed to add peer");
        return;
    }

    // Create tasks
    // Parameter #5 is the priority of the task
    xTaskCreate(vTaskReadButtonValue, "Check Button Value", 2048, NULL, 1, NULL);
    xTaskCreate(vTaskReceiveData, "Receive Data", 2048, NULL, 1, &vTaskReceiveDataHandle);
}
