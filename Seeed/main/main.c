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

/**
 * TODO:
 * - There is two different devices
 * - The devices are grouped into clusters with for example 10 Seeed devices and 1 LoRa device.
 * - Every cluster has one LoRa device
 * - The devices should be connected with BLE mesh or WiFi and ESP-NOW
 * - The LoRa device also have one button
 * - It will always check if the button is pressed or not
 *
 * - The Seeed devices are connected to one button.
 * - It will always check if the button is pressed or not
 * - If the button is pressed, then the Seeed device sends a message to the LoRa device telling the system
 * that the object (lifbuoy) have been taken from it's place.
 *
 * - As soon as the LoRa device receives the message, it will send a message via LoRa to a database
 * - The LoRa device should be able to recieve multiple messages from different Seeed devices at the same time and also from its' own button
 * - When a button is pressed down again, the LoRa device should recieve a notification and then send a message to the database to tell that the object is back in place
 *
 * - The LoRa device should also be able to send a message to the database one time everyday to tell that it is still alive and that all the Seeed devices are still connected
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

// MAC Address of responder - edit as required
uint8_t broadcastAddress[] = {0x78, 0x21, 0x84, 0x9E, 0xFE, 0xCC};

// Define a data structure
typedef struct struct_message
{
    char a[32];
    int id;
    int buttonValue;
} struct_message;

// Create a structured object
struct_message data;

// Peer info
esp_now_peer_info_t peerInfo;

// *** SET ID HERE ***
// *
int id = 1;
static const char* TAG = "MyModule";
// *
// *******************

// Source: https://github.com/espressif/esp-now/blob/master/examples/get-started/main/app_main.c
// TODO: Check provisioner vs. responder: https://github.com/espressif/esp-now/blob/master/examples/provisioning/main/app_main.c
// TODO: Byt namn till wifi_init ?
static void app_wifi_init()
{
    // TODO: Here or in app_main?
    nvs_flash_init(); // Not in the example above but necessary to prevent errors

    esp_event_loop_create_default();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    uint8_t proto = 0;
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE)); 
    esp_wifi_set_protocol(WIFI_IF_STA, WIFI_PROTOCOL_LR);
    printf("test");
    esp_wifi_get_protocol(WIFI_IF_STA, &proto);
    ESP_LOGI(TAG, "%d", proto);
}

// Alternative from Arduino:
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
    printf("\r\nLast Packet Send Status:\t");
    printf(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

// TODO: Change to vTaskFunction
void sendData()
{
    data.id = id;

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
            sendData();
        }

        // printf("Task Check Button Value is running: %lld\n", esp_timer_get_time() / 1000);
        // vTaskDelay(1000 / portTICK_PERIOD_MS);
        vTaskDelay(1); // 1 == 10 ms, to avoid watchdog timer to get triggered, TODO: Check if this is a good idea
    }
}

// TODO: Change sendData() function to vTask???
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

void app_main()
{
    // Set up Serial Monitor
    // Serial.begin(115200);
    // TODO: Är detta uart config här: https://github.com/espressif/esp-now/blob/master/examples/get-started/main/app_main.c

    // Setup pin to button
    // pinMode(D10, INPUT);
    gpio_set_direction(BUTTON_PIN, GPIO_MODE_INPUT);


    // Set ESP32 as a Wi-Fi Station
    // WiFi.mode(WIFI_STA);
    app_wifi_init();

    // Initilize ESP-NOW
    if (esp_now_init() != ESP_OK)
    {
        printf("Error initializing ESP-NOW");
        return;
    }

    // Register the send callback
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

    // Create tasks
    // Parameter #5 is the priority of the task
    xTaskCreate(vTaskReadButtonValue, "Check Button Value", 2048, NULL, 1, NULL); 
}
