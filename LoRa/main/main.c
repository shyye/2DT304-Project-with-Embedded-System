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


void vTask1(void *pvParameters)
{
    for (size_t i = 0; i < 5; i++)
    {
        printf("Task1 %d is running: %lld\n", i, esp_timer_get_time() / 1000);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}

void vTask2(void *pvParameters)
{
    for (size_t i = 0; i < 5; i++)
    {
        printf("Task2 %d is running: %lld\n", i, esp_timer_get_time() / 1000);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}

// void app_main(void)
// {
//     printf("\nTimer output in microseconds program initiation: %lld\n\n", esp_timer_get_time() / 1000);

//     // Parameter #5 is the priority of the task
//     xTaskCreate(vTask1, "Task 1", 2048, NULL, 1, NULL);
//     xTaskCreate(vTask2, "Task 2", 2048, NULL, 1, NULL);
// }

// Define a data structure
typedef struct struct_message
{
    char a[32];
    int id;
    int buttonValue;
} struct_message;

// Create a structured object
struct_message data;



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


// Callback function executed when data is received
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len)
{
    memcpy(&data, incomingData, sizeof(data));
    printf("Bytes received: %d\n", len);
    printf("Data received: %s\n", data.a);
    printf("ID of the sender: %d\n", data.id);
    printf("Button value: %d\n", data.buttonValue);
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
        int newButtonValue = 1; // TODO: Read from GPIO

        // Store in data.buttonValue
        data.buttonValue = newButtonValue;

        printf("Task Check Button Value is running: %lld\n", esp_timer_get_time() / 1000);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

/**
 * Send data to database via LoRa
 */
void vTaskSendDataToDatabase(void *pvParameters)
{
    for (;;)
    {
        if (data.buttonValue == 0)
        {
            // Send data to database
        }
    }
    vTaskDelete(NULL);
}

/**
 * Callback function when data is received via bluetooth or WiFi from Seeed devices
 * Maybe this should just be an ordinary fumction and not a task
 */
void vTaskReceiveData(void *pvParameters)
{
    for (;;)
    {
        // Receive data
        printf("Task Receive Data is running: %lld\n", esp_timer_get_time() / 1000);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}





void app_main(void)
{
    
    
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



    // TODO: behövs denna?
    // for (;;)
    // {
    //     // Loop forever
    // }
}