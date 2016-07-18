/* http_get - Retrieves a web page over HTTP GET.
 *
 * See http_get_ssl for a TLS-enabled version.
 *
 * This sample code is in the public domain.,
 */
#include "espressif/esp_common.h"
#include "esp/uart.h"

#include <string.h>

#include "FreeRTOS.h"
#include "task.h"

#include "mongoose.h"

//#include "lwip/err.h"
//#include "lwip/sockets.h"
//#include "lwip/sys.h"
//#include "lwip/netdb.h"
//#include "lwip/dns.h"

#include "sys/types.h"

#include "ssid_config.h"

#define WEB_SERVER "chainxor.org"
#define WEB_PORT 80
#define WEB_URL "http://chainxor.org/"

#define MG_TASK_STACK_SIZE 4096
#define MG_TASK_PRIORITY 1

void ev_handler(struct mg_connection *nc, int ev, void *ev_data) {
  struct http_message *hm = (struct http_message *) ev_data;

  switch (ev) {
    case MG_EV_CONNECT:
      if (*(int *) ev_data != 0) {
        fprintf(stderr, "connect() failed: %s\n", strerror(*(int *) ev_data));
      }
      break;
    case MG_EV_HTTP_REPLY:
      nc->flags |= MG_F_CLOSE_IMMEDIATELY;
        fwrite(hm->message.p, 1, hm->message.len, stdout);
      putchar('\n');
      break;
    default:
      break;
  }
}

static void mg_task(void *arg) {
    struct mg_mgr mgr;
    struct mg_connection *nc;

    uint8_t status = -1;
    while(status != STATION_GOT_IP) {
        status = sdk_wifi_station_get_connect_status();
        vTaskDelay(1000/portTICK_RATE_MS);
    }
    printf("Connection done and got ip!\n");

    mg_mgr_init(&mgr, NULL);

    nc = mg_connect_http(&mgr, ev_handler, WEB_URL, NULL, NULL);
    mg_set_protocol_http_websocket(nc);
    while (1) {
        mg_mgr_poll(&mgr, 1000);
    }
}

void user_init(void)
{
    uart_set_baud(UART0, 115200);
    printf("Running mongoose http example\nSDK version: %s", sdk_system_get_sdk_version());
    //printf("Hello world!\n");
    struct sdk_station_config config = {
        .ssid = WIFI_SSID,
        .password = WIFI_PASS
    };

    /* required to call wifi_set_opmode before station_set_config */
    sdk_wifi_set_opmode(STATION_MODE);
    sdk_wifi_station_set_config(&config);

    xTaskCreate(&mg_task, (signed char *)"mongoose", MG_TASK_STACK_SIZE, NULL, MG_TASK_PRIORITY, NULL);
}

