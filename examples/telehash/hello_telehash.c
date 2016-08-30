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

#include "telehash.h"

#ifdef WIFI_SSID
#undef WIFI_SSID
#endif

#ifdef WIFI_PASS
#undef WIFI_PASS
#endif


//#define WIFI_SSID "IC"
//#define WIFI_PASS "icomputacaoufal"
#define WIFI_SSID "GVT-C3C5"
#define WIFI_PASS "5403000556"

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

void user_init(void)
{
    uart_set_baud(UART0, 115200);
    struct sdk_station_config config = {
        .ssid = WIFI_SSID,
        .password = WIFI_PASS,
    };

    /* required to call wifi_set_opmode before station_set_config */
    sdk_wifi_set_opmode(STATIONAP_MODE);
    sdk_wifi_station_set_config(&config);


    mesh_t mesh = mesh_new();
    mesh_generate(mesh);
    printf("My id info is %s\n",mesh_json(mesh));

    xTaskCreate(blinkTicTask, (signed char *)"blinkTicTask", 256, NULL, 3, NULL);
}
