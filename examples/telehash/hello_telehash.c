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

#define __vTaskDelayMs(x) vTaskDelay(x/portTICK_RATE_MS)

mesh_t mesh;
lob_t secrets;

static uint8_t status = 0;

void link_check(link_t link)
{
  status = link_up(link) ? 1 : 0;
}

void telehashTask(void *pvParameters)
{
    lob_t id;
    mesh_t mesh;
    net_udp4_t udp4;

    mesh = mesh_new();
    mesh_generate(mesh);
    mesh_on_discover(mesh,"auto",mesh_add); // accept anyone
    mesh_on_link(mesh, "test", link_check); // testing the event being triggered
    status = 0;

    udp4 = net_udp4_new(mesh, NULL);

    id = mesh_json(mesh);
    printf("%s\n",lob_json(id));
    fflush(stdout);

    while(net_udp4_receive(udp4) && !status) {
        __vTaskDelayMs(100);
    }
    vTaskDelete(NULL);
}
void blinkTicTask(void *pvParameters)
{
    uint8_t status = 0;
    while(status != STATION_GOT_IP) {
        status = sdk_wifi_station_get_connect_status();
        __vTaskDelayMs(1000);
    }
    printf("Connection done and got ip!\r\n");

    mesh = mesh_new();
    secrets = mesh_generate(mesh);
    mesh_on_discover(mesh, (char *) "test", discover_handle);
    LOG_INFO("My id info is %s\n",hashname_char(mesh->id));
    LOG_INFO("Json secrets: %s\n", lob_json(secrets));

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
    sdk_wifi_set_opmode(STATIONAP_MODE);
    sdk_wifi_station_set_config(&config);

    xTaskCreate(blinkTicTask, (signed char *)"blinkTicTask", 1024, NULL, 3, NULL);
}
