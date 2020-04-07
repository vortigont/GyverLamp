/*
 *  Gyverlamp project  "Многофункциональный RGB светильник"
 *  Description      : defines and prototypes for main sketch
 */

#define MQTT_MAX_PACKET_SIZE 1024

#define DEFAULT_BRIGHNESS   50
#define DEFAULT_SPEED       30
#define DEFAULT_SCALE       40
#define DEFAULT_TIMER       60000   //msec

#define NTP_SERVER COUNTRY ".pool.ntp.org"

// NTP time
#include <coredecls.h>                  // settimeofday_cb()
#include <TZ.h>
#include <sntp.h>
