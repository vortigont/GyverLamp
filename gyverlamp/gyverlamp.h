/*
 *  Gyverlamp project  "Многофункциональный RGB светильник"
 *  Description      : defines and prototypes for main sketch
 */


#define NUM_LEDS WIDTH * HEIGHT
#define MQTT_MAX_PACKET_SIZE 1024

#define DEFAULT_BRIGHTNESS 128
#define DEFAULT_SPEED       30
#define DEFAULT_SCALE       40
#define TIMER_DEMO          60   // sec, время переключения эффектов в "Демо" режиме
#define TIMER_MQTT          60   // sec, время обновления статусов MQTT
#define TIMER_SCROLLER     120   // String scroller period, ms


#define NTP_SERVER COUNTRY ".pool.ntp.org"

#define BAUD                115200  // serial speed



// NTP time
#include <coredecls.h>                  // settimeofday_cb()
#include <TZ.h>
#include <sntp.h>

