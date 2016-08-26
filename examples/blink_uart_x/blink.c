/* The classic "blink" example
 *
 * This sample code is in the public domain.
 */
#include "espressif/esp_common.h"
#include "esp/uart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
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

void blinkTicTacTask(void *pvParameters)
{
    xQueueHandle xQueue = pvParameters;
    uint8_t flag = 0;
    while(1) {
        vTaskDelay(5000/portTICK_RATE_MS);
        flag = !flag;
        if(xQueueSend(xQueue, &flag,(portTickType) 11) == pdFALSE) {
            // TODO:
        }
    }
    vTaskDelete(NULL);
}

void blinkPrinterTask(void *pvParameters)
{
    xQueueHandle xQueue = pvParameters;
    uint8_t flag = 0;
    while(1) {
        //Receiving queue item;
        printf("\n");
        if(xQueue != 0) {
            if(xQueueReceive(xQueue, &flag , (portTickType) 100 ) == pdTRUE) {
                printf(" %s - %u \n", flag ? "tic": "tac", xTaskGetTickCount());
            } else {
                // TODO:
            }

        }

    }
    vTaskDelete(NULL);
}

void user_init(void)
{
    uart_set_baud(UART0, 115200);
    xQueueHandle xQueue = xQueueCreate( 1, sizeof( unsigned long ) );
    xTaskCreate(blinkTicTacTask, (signed char *)"blinkTicTacTask", 256, xQueue, 3, NULL);
    xTaskCreate(blinkPrinterTask, (signed char *)"blinkPrinterTask", 256, xQueue, 2, NULL);
}
