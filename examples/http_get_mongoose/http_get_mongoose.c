/* http_get - Retrieves a web page over HTTP GET.
 *
 * See http_get_ssl for a TLS-enabled version.
 *
 * This sample code is in the public domain.,
 */
#include "espressif/esp_common.h"
#include "esp/uart.h"

/* Go inside and change the PASSWORD and SSID for your connection. */
#include "ssid_config.h"

#include "FreeRTOS.h"
#include "task.h"

#include "mongoose.h"


#define WEB_SERVER "chainxor.org"
#define WEB_PORT 80
#define WEB_URL "http://chainxor.org/"

#define MG_TASK_STACK_SIZE 4096
#define MG_TASK_PRIORITY 1
#define MG_REQUESTER_TASK_PRIORITY 2

#define __vTaskDelayMs(x) vTaskDelay(x/portTICK_RATE_MS)

struct mg_mgr mongoose_event_manager;

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

static void do_request(void *arg) {
    struct mg_connection *nc;
    while (1) {
        nc = mg_connect_http(&mongoose_event_manager, ev_handler, WEB_URL, NULL, NULL);
        mg_set_protocol_http_websocket(nc);
        for(int countdown = 10; countdown >= 0; countdown--) {
            printf("%d... ", countdown);
            __vTaskDelayMs(1000);
        }
        printf("\r\nStarting again!\r\n");
    }
}

static void mg_task(void *arg) {    
    uint8_t status = -1;
    while(status != STATION_GOT_IP) {
        status = sdk_wifi_station_get_connect_status();
        __vTaskDelayMs(1000);
    }
    printf("Connection done and got ip!\r\n");
    mg_mgr_init(&mongoose_event_manager, NULL);
    xTaskCreate(&do_request, (signed char *)"requester", MG_TASK_STACK_SIZE, NULL,
                MG_REQUESTER_TASK_PRIORITY, NULL);
    while (1) {
        mg_mgr_poll(&mongoose_event_manager, 1000);
    }
}

void user_init(void)
{
    uart_set_baud(UART0, 115200);
    printf("Running mongoose http example\r\nSDK version: %s\r\n", sdk_system_get_sdk_version());
    struct sdk_station_config config = {
        .ssid = WIFI_SSID,
        .password = WIFI_PASS
    };

    /* required to call wifi_set_opmode before station_set_config */
    sdk_wifi_set_opmode(STATION_MODE);
    sdk_wifi_station_set_config(&config);

    xTaskCreate(&mg_task, (signed char *)"mongoose", MG_TASK_STACK_SIZE, NULL, MG_TASK_PRIORITY, NULL);
}

