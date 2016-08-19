/* The classic "blink" example
 *
 * This sample code is in the public domain.
 */
#include "espressif/esp_common.h"
#include "esp/uart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "esp8266.h"
#include <stdio.h>

#ifdef WIFI_SSID
#undef WIFI_SSID
#endif

#ifdef WIFI_PASS
#undef WIFI_PASS
#endif


#define WIFI_SSID "IC"
#define WIFI_PASS "icomputacaoufal"
//#define WIFI_SSID "GVT-C3C5"
//#define WIFI_PASS "5403000556"

void blinkTicTask(void *pvParameters)
{
    portTickType lastTick = xTaskGetTickCount();
    const uint32_t period = 2000 / portTICK_RATE_MS;
    while(1) {
        vTaskDelayUntil(&lastTick, period);
        printf("+ tic %u, %u\n", lastTick, xTaskGetTickCount());
    }
    vTaskDelete(NULL);
}

void blinkTacTask(void *pvParameters)
{
    portTickType lastTick = xTaskGetTickCount();
    const uint32_t period = 2000 / portTICK_RATE_MS;
    vTaskDelay(100);
    while(1) {
        vTaskDelayUntil(&lastTick, period);
        printf("- tac %u, %u\n", lastTick, xTaskGetTickCount());
    }
    vTaskDelete(NULL);
}

void user_init(void)
{
    uart_set_baud(UART0, 115200);
    vTaskDelay(300);
    xTaskCreate(blinkTicTask, (signed char *)"blinkTicTask", 256, NULL, 3, NULL);
    vTaskDelay(100);
    xTaskCreate(blinkTacTask, (signed char *)"blinkTacTask", 256, NULL, 3, NULL);
}
