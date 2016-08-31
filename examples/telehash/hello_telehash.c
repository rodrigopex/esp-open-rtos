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
#include "unit_test.h"


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

#define A_KEY "anfpjrveyyloypswpqzlfkjpwynahohffy"
#define A_SEC "cgcsbs7yphotlb5fxls5ogy2lrc7yxbg"
#define B_KEY "amhofcnwgmolf3owg2kipr5vus7uifydsy"
#define B_SEC "ge4i7h3jln4kltngwftg2yqtjjvemerw"

#define __pchar(x) (char *) x

tmesh_t netA = NULL, netB = NULL;
#define RXTX(a,b) (a->tx)?memcpy(b->frame,a->frame,64):memcpy(a->frame,b->frame,64)

#define __vTaskDelayMs(x) vTaskDelay(x/portTICK_RATE_MS)

tempo_t driver_sort(tmesh_t tm, tempo_t a, tempo_t b)
{
  if(a) return a;
  return b;
}

uint8_t scheduled = 0;
tmesh_t driver_schedule(tmesh_t tm)
{
  // start knock
  scheduled++;
  return tm;
}

tmesh_t driver_advance(tmesh_t tm, tempo_t tempo, uint8_t seed[8])
{
  // set channel and advance at based on seed
  tempo->at++;
  tempo->chan++;
  return tm;
}

tmesh_t driver_medium(tmesh_t tm, tempo_t tempo, uint8_t seed[8], uint32_t medium)
{
  tempo->driver = (void*)1; // flag for test check
  tempo->medium = medium?medium:1;
  return tm;
}

tmesh_t driver_free(tmesh_t tm, tempo_t tempo)
{
  return tm;
}

void tMeshTestTask(void *pvParameters)
{
    uint8_t status = 0;
    while(status != STATION_GOT_IP) {
        status = sdk_wifi_station_get_connect_status();
        __vTaskDelayMs(1000);
    }
    printf("Connection done and got ip!\r\n");

    fail_unless(!e3x_init(NULL)); // random seed

    mesh_t meshA = mesh_new();
    fail_unless(meshA);
    lob_t keyA = lob_new();
    lob_set(keyA, __pchar("1a"),A_KEY);
    lob_t secA = lob_new();
    lob_set(secA,__pchar("1a"),A_SEC);
    fail_unless(!mesh_load(meshA,secA,keyA));
    mesh_on_discover(meshA,"auto",mesh_add);

    lob_t keyB = lob_new();
    lob_set(keyB,__pchar("1a"),B_KEY);
    hashname_t hnB = hashname_vkeys(keyB);
    fail_unless(hnB);
    link_t linkAB = link_get(meshA,hnB);
    fail_unless(linkAB);

    netA = tmesh_new(meshA, (char *) "test", NULL);
    fail_unless(netA);

    netA->sort = driver_sort;
    netA->schedule = driver_schedule;
    netA->advance = driver_advance;
    netA->medium = driver_medium;
    netA->free = driver_free;

    fail_unless(netA->knock);
    fail_unless(strcmp(netA->community,"test") == 0);

    // create outgoing beacon
    fail_unless(tmesh_schedule(netA,1));
    fail_unless(netA->beacon);
    fail_unless(!netA->beacon->frames);
    fail_unless(!netA->beacon->mote);
    fail_unless(netA->beacon->medium == 1);

    // should have schedule a beacon rx
    fail_unless(scheduled == 1);
    fail_unless(netA->knock->is_active);
    fail_unless(netA->knock->tempo == netA->beacon);
    fail_unless(netA->knock->tempo->at == 2);
    fail_unless(netA->knock->tempo->chan == 1);

    mote_t moteB = tmesh_mote(netA, linkAB);
    fail_unless(moteB);
    fail_unless(moteB->link == linkAB);
    fail_unless(moteB->signal);
    fail_unless(moteB->signal->medium == 1);
    fail_unless(moteB->signal->driver == (void*)1);
    mesh_free(meshA);
    tmesh_free(netA);
    tmesh_free(netB);
    printf("The end of the testes. Well done!\n");
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

    xTaskCreate(tMeshTestTask, (signed char *)"blinkTicTask", 1024, NULL, 3, NULL);
}
