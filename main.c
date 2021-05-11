#include <stdio.h>
#include <string.h>

#include "thread.h"
#include "xtimer.h"

#include "ccs811.h"
#include "ccs811_params.h"

#include "msg.h"
#include "thread.h"
#include "fmt.h"

#include "periph/rtc.h"

#include "net/loramac.h"
#include "semtech_loramac.h"

/* Messages are sent every 20s to respect the duty cycle on each channel */
#define PERIOD              (20U)
#define TEMPERATURE 20
#define HUMIDITY 50

typedef struct { // à mettre dans un .h, penser à mettre tout les prototypes des fonctions
  uint16_t co2;
  uint16_t Tvoc;
} sensor_data;

void sensor_data_init(sensor_data){ // à mettre dans un .c de fonctions
  sensor_data.co2=0;
  sensor_data.Tvoc=0;
}

semtech_loramac_t loramac;

static const char *message = "This is RIOT!";

static uint8_t deveui[LORAMAC_DEVEUI_LEN];
static uint8_t appeui[LORAMAC_APPEUI_LEN];
static uint8_t appkey[LORAMAC_APPKEY_LEN];


static void _prepare_next_alarm(void)
{
    struct tm time;
    rtc_get_time(&time);
    /* set initial alarm */
    time.tm_sec += PERIOD;
    mktime(&time);
    rtc_set_alarm(&time, rtc_cb, NULL);
}

static void _send_message(sensor_data data)
{
    printf("Sending:); // à compléter
    /* Try to send the message */
    uint8_t ret = semtech_loramac_send(&loramac,(uint8_t *)message, strlen(message));
    if (ret != SEMTECH_LORAMAC_TX_DONE)  {
        printf("Cannot send message '%s', ret code: %d\n", message, ret);
        return;
    }
}

int connection_setup(void)
{
    puts("Transmission of gas data");
    puts("=====================================");

    /* Convert identifiers and application key */
    fmt_hex_bytes(deveui, DEVEUI);
    fmt_hex_bytes(appeui, APPEUI);
    fmt_hex_bytes(appkey, APPKEY);

    /* Initialize the loramac stack */
    semtech_loramac_init(&loramac);
    semtech_loramac_set_deveui(&loramac, deveui);
    semtech_loramac_set_appeui(&loramac, appeui);
    semtech_loramac_set_appkey(&loramac, appkey);

    /* Use a fast datarate, e.g. BW125/SF7 in EU868 */
    semtech_loramac_set_dr(&loramac, LORAMAC_DR_5);

    /* Start the Over-The-Air Activation (OTAA) procedure to retrieve the
     * generated device address and to get the network and application session
     * keys.
     */
    puts("Starting join procedure");
    if (semtech_loramac_join(&loramac, LORAMAC_JOIN_OTAA) != SEMTECH_LORAMAC_JOIN_SUCCEEDED) {
        puts("Join procedure failed");
        return 1;
    }
    puts("Join procedure succeeded");

}


int main(void)
{
    ccs811_t sensor;
    xtimer_sleep(10);
    puts("CCS811 test application\n");

    puts("+------------Initializing------------+\n");
    /* initialize the sensor with default configuration parameters */

    if (ccs811_init (&sensor, &ccs811_params[0]) != CCS811_OK) {

        puts("Initialization failed\n");
        return 1;
    }

    puts("Setting up parameters\n");

    ccs811_set_environmental_data (&sensor, TEMPERATURE, HUMIDITY);

    printf("\n+--------Starting Measurements--------+\n");

    while (1) {
        sensor_data container;

        if (ccs811_power_up(&sensor) != CCS881_OK) {
          puts("Could not power up the device");
        }

        while(ccs811_data_ready(&sensor) != CCS811_OK){}
        /* wait and check for for new data every 10 ms */
        //while (ccs811_data_ready (&sensor) != CCS811_OK) {
        	//puts("ready failure\n");
         //   xtimer_sleep(2);
        //}

        /* read the data and print them on success */
        if (ccs811_read_iaq(&sensor,&tvoc, &eco2, 0, 0) != CCS811_OK) {
            printf("TVOC [ppb]: %d\neCO2 [ppm]: %d\n", tvoc, eco2);
            puts("+-------------------------------------+");
        }
        else {
            printf("Could not read data from sensor\n");
        }

    if (ccs811_power_down(&sensor) != CCS881_OK) {
      puts("Could not power down the device");
    }


    }//end of while
    msg

    return 0;
}
