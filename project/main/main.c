/*
 * REFERENCES: 
 * 
 * Two tasks - FreeRTOS ESP IDF for esp32 by SIMS IOT DEVICES
 * https://www.youtube.com/watch?v=bEhZp4Adghc
 * 
 * 
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h" // TODO: Check if this is needed
#include "esp_timer.h"

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

void app_main(void)
{
    printf("\nTimer output in microseconds program initiation: %lld\n\n", esp_timer_get_time() / 1000);

    // Parameter #5 is the priority of the task
    xTaskCreate(vTask1, "Task 1", 2048, NULL, 1, NULL);
    xTaskCreate(vTask2, "Task 2", 2048, NULL, 1, NULL);
}