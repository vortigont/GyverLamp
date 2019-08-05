/*
  Скетч к проекту "Многофункциональный RGB светильник"
  Страница проекта (схемы, описания): https://alexgyver.ru/GyverLamp/
  Исходники на GitHub: https://github.com/AlexGyver/GyverLamp/
  Нравится, как написан код? Поддержи автора! https://alexgyver.ru/support_alex/
  Автор: AlexGyver, AlexGyver Technologies, 2019
  https://AlexGyver.ru/
*/

/*
  Версия 1.4:
  - Исправлен баг при смене режимов
  - Исправлены тормоза в режиме точки доступа

  Версия 1.4 MQTT Edition:
  - Удалена настройка статического IP - статический IP лучше настраивать на стороне роутера
  - Добавлена поддержка MQTT сервера
  - Добавлено ОТА обновление через сетевой порт
  - Добавлена интеграция с Home Assistant через MQTT Discover - лампа просто появится в Home Assistant
  - Добавлена возможность выбирать цвет из RGB палитры HomeAssistant
  - Добавлено автообнаружение подключения кнопки
  - Добавлен запуск портала конфигурации при неудачном подключении к WiFi сети
  - Добавлено адаптивное подключение к MQTT серверу в случае потери соединениия
  - Добавлено принудительное включение эффекта матрицы во время OTA обновлении

*/

// Ссылка для менеджера плат:
// http://arduino.esp8266.com/stable/package_esp8266com_index.json

// ============= НАСТРОЙКИ =============
// -------- ВРЕМЯ -------
#define GMT 5              // смещение (москва 3)
#define NTP_ADDRESS  "europe.pool.ntp.org"    // сервер времени

// -------- РАССВЕТ -------
#define DAWN_BRIGHT 200       // макс. яркость рассвета
#define DAWN_TIMEOUT 1        // сколько рассвет светит после времени будильника, минут

// ---------- МАТРИЦА ---------
#define BRIGHTNESS 40         // стандартная маскимальная яркость (0-255)
#define CURRENT_LIMIT 2000    // лимит по току в миллиамперах, автоматически управляет яркостью (пожалей свой блок питания!) 0 - выключить лимит

#define WIDTH 16              // ширина матрицы
#define HEIGHT 11             // высота матрицы

#define COLOR_ORDER GRB       // порядок цветов на ленте. Если цвет отображается некорректно - меняйте. Начать можно с RGB

#define MATRIX_TYPE 0         // тип матрицы: 0 - зигзаг, 1 - параллельная
#define CONNECTION_ANGLE 0    // угол подключения: 0 - левый нижний, 1 - левый верхний, 2 - правый верхний, 3 - правый нижний
#define STRIP_DIRECTION 1     // направление ленты из угла: 0 - вправо, 1 - вверх, 2 - влево, 3 - вниз
// при неправильной настройке матрицы вы получите предупреждение "Wrong matrix parameters! Set to default"
// шпаргалка по настройке матрицы здесь! https://alexgyver.ru/matrix_guide/

// --------- ESP --------
#define ESP_MODE 1
// 0 - точка доступа
// 1 - локальный
byte IP_AP[] = {192, 168, 4, 100};   // статический IP точки доступа (менять только последнюю цифру)

// ----- AP (точка доступа) -------
#define AP_SSID "GyverLamp"
#define AP_PASS "12345678"
#define AP_PORT 8888

// ============= ДЛЯ РАЗРАБОТЧИКОВ =============
#define LED_PIN 2             // пин ленты
#define BTN_PIN 4
#define MODE_AMOUNT 18

#define NUM_LEDS WIDTH * HEIGHT
#define SEGMENTS 1            // диодов в одном "пикселе" (для создания матрицы из кусков ленты)
// ---------------- БИБЛИОТЕКИ -----------------
#define FASTLED_INTERRUPT_RETRY_COUNT 0
#define FASTLED_ALLOW_INTERRUPTS 0
#define FASTLED_ESP8266_RAW_PIN_ORDER
#define NTP_INTERVAL 600 * 1000    // обновление (1 минута)

//#define DEBUG

#include "timerMinim.h"
#include <FastLED.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiClient.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>
#include <WiFiManager.h>
#include <WiFiUdp.h>
#include <EEPROM.h>
#include <NTPClient.h>
#include <GyverButton.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <ArduinoOTA.h>

// ------------------- ТИПЫ --------------------

CRGB leds[NUM_LEDS];
WiFiUDP Udp;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_ADDRESS, GMT * 3600, NTP_INTERVAL);
timerMinim timeTimer(3000);
GButton touch(BTN_PIN, LOW_PULL, NORM_OPEN);
ESP8266WebServer *http; // запуск слушателя 80 порта (эйкей вебсервер)

// ----------------- ПЕРЕМЕННЫЕ ------------------

const char AP_NameChar[] = AP_SSID;
const char WiFiPassword[] = AP_PASS;
unsigned int localPort = AP_PORT;
char packetBuffer[UDP_TX_PACKET_MAX_SIZE + 1]; //buffer to hold incoming packet
String inputBuffer;
static const byte maxDim = max(WIDTH, HEIGHT);

struct {
  byte brightness = 50;
  byte speed = 30;
  byte scale = 40;
} modes[MODE_AMOUNT];

byte r = 255;
byte g = 255;
byte b = 255;

struct {
  boolean state = false;
  int time = 0;
} alarm[7];

byte dawnOffsets[] = {5, 10, 15, 20, 25, 30, 40, 50, 60};
byte dawnMode;
boolean dawnFlag = false;
long thisTime;
boolean manualOff = false;

int8_t currentMode = 0;
boolean loadingFlag = true;
boolean ONflag = true;
uint32_t eepromTimer;
boolean settChanged = false;
// Конфетти, Огонь, Радуга верт., Радуга гориз., Смена цвета,
// Безумие 3D, Облака 3D, Лава 3D, Плазма 3D, Радуга 3D,
// Павлин 3D, Зебра 3D, Лес 3D, Океан 3D,

unsigned char matrixValue[8][16];

WiFiClient espClient;
PubSubClient mqttclient(espClient);

// ID клиента, менять для интеграции с системами умного дома в случае необходимости
String clientId = "ESP-"+String(ESP.getChipId(), HEX);

bool USE_MQTT = true; // используем  MQTT?
bool _BTN_CONNECTED = true; 

struct MQTTconfig {
  char HOST[32];
  char USER[32];
  char PASSWD[32];
};

bool shouldSaveConfig = false;

void saveConfigCallback () {
  Serial.println("should save config");
  shouldSaveConfig = true;
}

void setup() {

  // ЛЕНТА
  FastLED.addLeds<WS2812B, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS)/*.setCorrection( TypicalLEDStrip )*/;
  FastLED.setBrightness(BRIGHTNESS);
  if (CURRENT_LIMIT > 0) FastLED.setMaxPowerInVoltsAndMilliamps(5, CURRENT_LIMIT);
  FastLED.clear();
  FastLED.show();

  touch.setStepTimeout(100);
  touch.setClickTimeout(500);

  Serial.begin(115200);
  Serial.println();
  
  EEPROM.begin(512);

  // WI-FI
  if (ESP_MODE == 0) {    // режим точки доступа
    WiFi.softAPConfig(IPAddress(IP_AP[0], IP_AP[1], IP_AP[2], IP_AP[3]),
                      IPAddress(192, 168, 4, 1),
                      IPAddress(255, 255, 255, 0));

    WiFi.softAP(AP_NameChar, WiFiPassword);
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("Access point Mode");
    Serial.println("AP IP address: ");
    Serial.println(myIP);
    USE_MQTT = false;

  } else {                // подключаемся к роутеру
    Serial.print("WiFi manager...");
    digitalWrite(LED_BUILTIN, HIGH);

    char mqtt_server[32] = "";
    char mqtt_user[32] = "DEVS_USER";
    char mqtt_password[32] = "DEVS_PASSWD";
    
    WiFiManager wifiManager;
    WiFiManagerParameter custom_mqtt_server("server", "mqtt server", mqtt_server, 40);
    WiFiManagerParameter custom_mqtt_username("user", "mqtt user", mqtt_user, 10);
    WiFiManagerParameter custom_mqtt_password("password", "mqtt password", mqtt_password, 10);
    
    wifiManager.setSaveConfigCallback(saveConfigCallback);
    wifiManager.setDebugOutput(false);

    wifiManager.addParameter(&custom_mqtt_server);
    wifiManager.addParameter(&custom_mqtt_username);
    wifiManager.addParameter(&custom_mqtt_password);

    if (!wifiManager.autoConnect()) {
      if (!wifiManager.startConfigPortal()) {
         Serial.println("failed to connect and hit timeout");
      }      
    }

    strcpy(mqtt_server, custom_mqtt_server.getValue());
    strcpy(mqtt_user, custom_mqtt_username.getValue());
    strcpy(mqtt_password, custom_mqtt_password.getValue());

    if (shouldSaveConfig) {
      
      writeMQTTConfig(mqtt_server, mqtt_user,mqtt_password);
      Serial.println("MQTT configuration written");
    };

    Serial.print("connected! IP address: ");
    Serial.print(WiFi.localIP());
    Serial.print(". Signal strength: ");
    Serial.print(2*(WiFi.RSSI()+100));
    Serial.println("%");
    digitalWrite(LED_BUILTIN, LOW);

    #ifdef DEBUG
    Serial.print("onChip memory size: ");
    Serial.print(ESP.getFlashChipSize()/1024/8);
    Serial.println("Kb");
    
    Serial.print("Free Heap size: ");
    Serial.print(ESP.getFreeHeap()/1024);
    Serial.println("Kb");
    #endif

    if (!MDNS.begin(clientId)) {
        Serial.println("Error setting up MDNS responder!");
    }
  
    ArduinoOTA.onStart([]() {
      Serial.println("OTA Start");
      ONflag = true;
      currentMode = 16;
      loadingFlag = true;
      FastLED.clear();
      FastLED.setBrightness(modes[currentMode].brightness);
      
    });
    
    ArduinoOTA.onEnd([]() {
      Serial.println("OTA End");  //  "Завершение OTA-апдейта"
    });
    
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
      effectsTick();
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    
    ArduinoOTA.onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed"); 
   });
    
    ArduinoOTA.begin(); 
  }
  
  Udp.begin(localPort);
  Serial.printf("UDP server on port %d\n", localPort);

  // EEPROM
  
  delay(50);
  
  if (EEPROM.read(198) != 20) {   // первый запуск
    EEPROM.write(198, 20);
    //EEPROM.commit();

    for (byte i = 0; i < MODE_AMOUNT; i++) {
      EEPROM.put(3 * i + 40, modes[i]);
      //EEPROM.commit();
    }
    for (byte i = 0; i < 7; i++) {
      EEPROM.write(5 * i, alarm[i].state);   // рассвет
      eeWriteInt(5 * i + 1, alarm[i].time);
      //EEPROM.commit();
    }
    EEPROM.write(199, 0);   // рассвет
    EEPROM.write(200, 0);   // режим
    
    EEPROM.commit();
  }

  for (byte i = 0; i < MODE_AMOUNT; i++) {
    EEPROM.get(3 * i + 40, modes[i]);
  }
  
  for (byte i = 0; i < 7; i++) {
    alarm[i].state = EEPROM.read(5 * i);
    alarm[i].time = eeGetInt(5 * i + 1);
  }
  
  dawnMode = EEPROM.read(199);
  currentMode = (int8_t)EEPROM.read(200);

  // отправляем настройки
  sendCurrent();
  char reply[inputBuffer.length() + 1];
  inputBuffer.toCharArray(reply, inputBuffer.length() + 1);
  Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
  Udp.write(reply);
  Udp.endPacket();

  timeClient.begin();
  memset(matrixValue, 0, sizeof(matrixValue));
  randomSeed(micros());
  webserver();
  MDNS.addService("http", "tcp", 80);

  MQTTconfig MQTTConfig = readMQTTConfig();
  
  if ((String(MQTTConfig.HOST) == "none") || (ESP_MODE == 0)) {

    USE_MQTT = false;
    Serial.println("Использование MQTT сервера отключено.");
  }

   _BTN_CONNECTED = !digitalRead(BTN_PIN);

  #ifdef DEBUG
  _BTN_CONNECTED ? Serial.println("Обнаружена сенсорная кнопка") : Serial.println("Cенсорная кнопка не обнаружена, управление сенсорной кнопкой отключено");
  #endif
  
}

void loop() {
  parseUDP();
  effectsTick();
  eepromTick();
  timeTick();
  buttonTick();
  MDNS.update();
  http->handleClient();

  if (USE_MQTT && !mqttclient.connected()) MQTTreconnect();
  if (USE_MQTT) mqttclient.loop();

  ArduinoOTA.handle();
  yield();
}

void eeWriteInt(int pos, int val) {
  byte* p = (byte*) &val;
  EEPROM.write(pos, *p);
  EEPROM.write(pos + 1, *(p + 1));
  EEPROM.write(pos + 2, *(p + 2));
  EEPROM.write(pos + 3, *(p + 3));
  EEPROM.commit();
}

int eeGetInt(int pos) {
  int val;
  byte* p = (byte*) &val;
  *p        = EEPROM.read(pos);
  *(p + 1)  = EEPROM.read(pos + 1);
  *(p + 2)  = EEPROM.read(pos + 2);
  *(p + 3)  = EEPROM.read(pos + 3);
  return val;
}
