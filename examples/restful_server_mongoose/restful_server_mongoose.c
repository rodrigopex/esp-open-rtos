/* restful_server_mongoose - Implements RESTFul server
 * using mongoose.
 *
 * Based on
 * https://github.com/cesanta/mongoose/tree/master/examples/restful_server
 *
 * Known issues: Memory leak when the request comes from browsers. From
 * Postman there is no leakage.
 *
 * This sample code is in the public domain.
 */
#include "espressif/esp_common.h"
#include "esp/uart.h"

/* Go inside and change the PASSWORD and SSID for your connection. */
#include "ssid_config.h"

#include "FreeRTOS.h"
#include "task.h"

#include "mongoose.h"

#define MG_TASK_STACK_SIZE 4096
#define MG_TASK_PRIORITY 1

#define __vTaskDelayMs(x) vTaskDelay(x/portTICK_RATE_MS)
#define __UNUSED(x) (void) x

struct mg_mgr mongoose_event_manager;
static const char *s_http_port = "8000";
static struct mg_serve_http_opts s_http_server_opts;

/* Example of request using data from query-string:
     - http://172.20.8.123:8000/api/v1/sum?n1=10&n2=50
*/
static void handle_sum_call(struct mg_connection *nc, struct http_message *hm) {
    char n1[10], n2[10];
    double result;

    //printf("Message %s", hm->message.p);

    if(strncmp("GET", hm->method.p, 1) == 0) {
        printf("GET Received\r\n");
    } if(strncmp("POST", hm->method.p, 2) == 0) {
        printf("POST Received\r\n");
    }

    /* Get form variables from query-string GET*/
    mg_get_http_var(&hm->query_string, "n1", n1, sizeof(n1));
    mg_get_http_var(&hm->query_string, "n2", n2, sizeof(n2));

    /* Send headers */
    mg_printf(nc, "%s", "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n");

    /* Compute the result and send it back as a JSON object */
    result = strtod(n1, NULL) + strtod(n2, NULL);
    mg_printf_http_chunk(nc, "{ \"result\": %lf }", result);
    mg_send_http_chunk(nc, "", 0); /* Send empty chunk, the end of response */
    nc->flags |= MG_F_SEND_AND_CLOSE;
}

static void ev_handler(struct mg_connection *nc, int ev, void *ev_data) {
    struct http_message *hm = (struct http_message *) ev_data;
    switch (ev) {

    case MG_EV_HTTP_REQUEST:
        if (mg_vcmp(&hm->uri, "/api/v1/sum") == 0) {
            handle_sum_call(nc, hm); /* Handle RESTful call */
        } else if (mg_vcmp(&hm->uri, "/printcontent") == 0) {
            char buf[100] = {0};
            memcpy(buf, hm->body.p,
                   sizeof(buf) - 1 < hm->body.len ? sizeof(buf) - 1 : hm->body.len);
            printf("%s\n", buf);
        } else {
            mg_serve_http(nc, hm, s_http_server_opts); /* Serve static content */
        }
        break;
//    case MG_EV_RECV:
//        mbuf_remove(&nc->recv_mbuf, (&nc->recv_mbuf.len - *((int *) ev_data)));
//        break;
    default:
        printf("Event: %d %ld\n", ev, nc->last_io_time);
        break;
    }
}

size_t currentFreeHeapSize = 0, previousFreeHeapSize = 0;
static void __checkMemory() {
    previousFreeHeapSize = currentFreeHeapSize;
    currentFreeHeapSize = xPortGetFreeHeapSize();
    if(previousFreeHeapSize != currentFreeHeapSize) {
        if(previousFreeHeapSize > currentFreeHeapSize) {
            printf("HeapSize [%ld]\r\n", currentFreeHeapSize);
        } else {
            printf("HeapSize [%ld]+ \r\n", currentFreeHeapSize);
        }
    }
}

static void mg_task(void *arg) {
    __UNUSED(arg);
    struct mg_connection *nc;
    uint8_t status = 200;
    while(status != STATION_GOT_IP) {
        status = sdk_wifi_station_get_connect_status();
        __vTaskDelayMs(1000);
    }
    printf("Connection done and got ip!\r\n");

    mg_mgr_init(&mongoose_event_manager, NULL);

    /* Set HTTP server options */
    nc = mg_bind(&mongoose_event_manager, s_http_port, ev_handler);

    if (nc == NULL) {
        fprintf(stderr, "Error starting server on port %s\n", s_http_port);
        return;
    }

    s_http_server_opts.document_root = "/";
    mg_set_protocol_http_websocket(nc);

    while (1) {
        mg_mgr_poll(&mongoose_event_manager, 1000);
        __checkMemory();
    }
    mg_mgr_free(&mongoose_event_manager);
    vTaskDelete(NULL);
}

void user_init(void)
{
    uart_set_baud(UART0, 115200);
    printf("At the beginning: [%ld]\n", xPortGetFreeHeapSize());
    printf("Running mongoose http example\r\nSDK version: %s\r\n", sdk_system_get_sdk_version());
    struct sdk_station_config config = { .ssid = WIFI_SSID, .password = WIFI_PASS };

    /* required to call wifi_set_opmode before station_set_config */
    sdk_wifi_set_opmode(STATION_MODE);
    sdk_wifi_station_set_config(&config);

    xTaskCreate(&mg_task, (signed char *)"mongoose", MG_TASK_STACK_SIZE, NULL, MG_TASK_PRIORITY,
                NULL);
}

