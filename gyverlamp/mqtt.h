#pragma once

#define MQTT_RECONNECT_TIMEOUT      5000
#define MQTT_RECONNECT_ATTEMPTS       10
#define MQTT_TOPIC_BUFF_LEN           48

#define ESP_ID_LEN 6

/*
 * PROGMEM char arrays with static data
 */

// Topic templates
static const char topic_light_id_config_P[] PROGMEM = "homeassistant/light/%s/config";
static const char topic_sensor_id_Wconfig_P[] PROGMEM = "homeassistant/sensor/%sW/config";
static const char topic_sensor_id_Uconfig_P[] PROGMEM = "homeassistant/sensor/%sU/config";

// debug messages
static const char dbg_conattempt_P[] PROGMEM = "Attempting MQTT connection to %s on port %s as %s...\n";
static const char dbg_publish_P[] PROGMEM = "Error publishing MQTT\n";
static const char dbg_publishcomplete_P[] PROGMEM = "Unable to complete MQTT-Publish\n";
static const char dbg_publishlong_P[] PROGMEM = "Publishing long msg %i bytes\n";

// hass discover chunks
#define HA_DISCOVER_PGMCHUNKS 7
#define HA_DISCOVER_IDCHUNKS 4
static const char HA_discover00[] PROGMEM = R"===({"~": "homeassistant/light/ESP-)===";
static const char HA_discover01[] PROGMEM = R"===(","name":"Gyver Lamp ESP-)===";
static const char HA_discover02[] PROGMEM = R"===(","uniq_id":")===";
static const char HA_discover03[] PROGMEM = R"===(",
"avty_t":"~/state",
"pl_avail":"online","pl_not_avail":"offline",
"bri_cmd_t":"~/brightness/set","bri_stat_t":"~/brightness/status","bri_scl":255,
"cmd_t":"~/switch",
"stat_t":"~/status",
"fx_cmd_t":"~/effect/set","fx_stat_t":"~/effect/status",
"rgb_cmd_t":"~/rgb/set","rgb_stat_t":"~/rgb/status",)===";
static const char HA_discover04[] PROGMEM = R"===("fx_list": ["Конфетти","Огонь","Радуга верт.","Радуга гориз.","Смена цвета","Безумие 3D","Облака 3D","Лава 3D","Плазма 3D","Радуга 3D","Павлин 3D","Зебра 3D","Лес 3D","Океан 3D","Цвет","Снегопад","Матрица","Светлячки","Аквариум","Звездопад","Пейнтбол","Спираль","Демо"],)===";
static const char HA_discover05[] PROGMEM = R"===("device":{"ids": [")===";
static const char HA_discover06[] PROGMEM = R"===("],"name":"Gyver Lamp","mf":"Alex Gyver","mdl":"Gyver Lamp v2","sw":"1.5.5 MQTT"}})===";

static const char* const HA_discover_dev[] PROGMEM = {HA_discover00, HA_discover01, HA_discover02, HA_discover03, HA_discover04, HA_discover05, HA_discover06};

// hass discover signal chunks
#define HA_DISCOVER_S_PGMCHUNKS 6
#define HA_DISCOVER_S_IDCHUNKS 5
static const char HA_discover_s00[] PROGMEM = R"===({"~": "homeassistant/sensor/ESP-)===";
static const char HA_discover_s01[] PROGMEM = R"===(W","device_class":"signal_strength","name":"Signal Strength ESP-)===";
static const char HA_discover_s02[] PROGMEM = R"===(","state_topic":"~/WiFi/RSSI_pct","unit_of_measurement":"%","uniq_id":"W)===";
static const char HA_discover_s03[] PROGMEM = R"===(","avty_t":"homeassistant/light/ESP-)===";
static const char HA_discover_s04[] PROGMEM = R"===(/state","pl_avail":"online","pl_not_avail":"offline","device":{"ids":[")===";
static const char HA_discover_s05[] PROGMEM = R"===("]}})===";

static const char* const HA_discover_sig[] PROGMEM = {HA_discover_s00, HA_discover_s01, HA_discover_s02, HA_discover_s03, HA_discover_s04, HA_discover_s05};

// hass discover uptime chunks
#define HA_DISCOVER_U_PGMCHUNKS 6
#define HA_DISCOVER_U_IDCHUNKS 5
// first chunk is the same as 'hass discover signal'
static const char HA_discover_u01[] PROGMEM = R"===(U","ic":"mdi:timer","name": "Uptime ESP-)===";
static const char HA_discover_u02[] PROGMEM = R"===(","state_topic":"~/uptime","unit_of_measurement":"s","uniq_id":"U)===";
// last 3 chunks are the same as 'hass discover signal'

static const char* const HA_discover_upt[] PROGMEM = {HA_discover_s00, HA_discover_u01, HA_discover_u02, HA_discover_s03, HA_discover_s04, HA_discover_s05};
