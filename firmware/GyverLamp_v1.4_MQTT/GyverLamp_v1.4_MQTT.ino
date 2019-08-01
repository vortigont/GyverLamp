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
  - Добавлена поддержка MQTT сервера
  - Добавлена интеграция с Home Assistant через MQTT Discover - лампа просто появится в Home Assistant
  - Добавлена возможность выбирать цвет из RGB палитры HomeAssistant

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
#define HEIGHT 16             // высота матрицы

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
byte IP_AP[] = {192, 168, 1, 100};   // статический IP точки доступа (менять только последнюю цифру)
//byte IP_STA[] = {192, 168, 1, 220};  // статический IP локальный (менять только последнюю цифру)

// ----- AP (точка доступа) -------
#define AP_SSID "GyverLamp"
#define AP_PASS "12345678"
#define AP_PORT 8888

// -------- Менеджер WiFi ---------
//#define AC_SSID "AutoConnectAP"
//#define AC_PASS "12345678"

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
#define NTP_INTERVAL 60 * 1000    // обновление (1 минута)

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
//WiFiServer server(80);
WiFiUDP Udp;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_ADDRESS, GMT * 3600, NTP_INTERVAL);
timerMinim timeTimer(3000);
GButton touch(BTN_PIN, LOW_PULL, NORM_OPEN);

// ----------------- ПЕРЕМЕННЫЕ ------------------
//const char* autoConnectSSID = AC_SSID;
//const char* autoConnectPass = AC_PASS;
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
bool USE_MQTT = true; // используем  MQTT?

char mqtt_server[32];
char mqtt_user[32];
char mqtt_password[32];

struct MQTTconfig {
  char HOST[32];
  char USER[32];
  char PASSWD[32];
};

// айдии клиента, менять для интеграции с системами умного дома в случае необходимости
String clientId = "ESP-"+String(ESP.getChipId(), HEX);

MQTTconfig readMQTTConfig () {
  int eeAddress = 300;
  MQTTconfig MQTTConfig;

  //EEPROM.begin(512);

  for (int i = 0; i < 32; ++i) MQTTConfig.HOST[i]   = char(EEPROM.read(eeAddress + i)); eeAddress += 32;
  for (int i = 0; i < 32; ++i) MQTTConfig.USER[i]   = char(EEPROM.read(eeAddress + i)); eeAddress += 32;
  for (int i = 0; i < 32; ++i) MQTTConfig.PASSWD[i] = char(EEPROM.read(eeAddress + i)); 

  //EEPROM.end();
  return MQTTConfig;
}

bool writeMQTTConfig(const char HOST[32], const char USER[32], const char PASSWD[32]) {
  int eeAddress = 300;

  //EEPROM.begin(512);

  //sizeof(HOST) > 0 ? USE_MQTT = true : USE_MQTT = false;
  //EEPROM.write(eeAddress, USE_MQTT); eeAddress +=1;

  for (int i = 0; i < 32; ++i)  EEPROM.write(eeAddress + i, HOST[i]); eeAddress += 32;
  for (int i = 0; i < 32; ++i)  EEPROM.write(eeAddress + i, USER[i]); eeAddress += 32;
  for (int i = 0; i < 32; ++i)  EEPROM.write(eeAddress + i, PASSWD[i]); 

  EEPROM.commit();
  //EEPROM.end();

  return true;
}

bool shouldSaveConfig = false;

void saveConfigCallback () {
  Serial.println("should save config");
  shouldSaveConfig = true;
}

void setup() {

  ESP.wdtDisable();
  //ESP.wdtEnable(WDTO_8S);

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

  // WI-FI
  if (ESP_MODE == 0) {    // режим точки доступа
    WiFi.softAPConfig(IPAddress(IP_AP[0], IP_AP[1], IP_AP[2], IP_AP[3]),
                      IPAddress(192, 168, 4, 1),
                      IPAddress(255, 255, 255, 0));

    WiFi.softAP(AP_NameChar, WiFiPassword);
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("Access point Mode");
    Serial.print("AP IP address: ");
    Serial.println(myIP);

    //server.begin();
  } else {                // подключаемся к роутеру
    Serial.print("WiFi manager...");
    
    WiFiManagerParameter custom_mqtt_server("server", "mqtt server", mqtt_server, 40);
    WiFiManagerParameter custom_mqtt_username("user", "mqtt user", mqtt_user, 10);
    WiFiManagerParameter custom_mqtt_password("password", "mqtt password", mqtt_password, 10);
    
    WiFiManager wifiManager;
    wifiManager.setSaveConfigCallback(saveConfigCallback);
    wifiManager.setDebugOutput(false);

    wifiManager.addParameter(&custom_mqtt_server);
    wifiManager.addParameter(&custom_mqtt_username);
    wifiManager.addParameter(&custom_mqtt_password);

    wifiManager.autoConnect();

    strcpy(mqtt_server, custom_mqtt_server.getValue());
    strcpy(mqtt_user, custom_mqtt_username.getValue());
    strcpy(mqtt_password, custom_mqtt_password.getValue());

    EEPROM.begin(512);

    if (shouldSaveConfig) {
      
      writeMQTTConfig(mqtt_server, mqtt_user,mqtt_password);
      Serial.println("MQTT config written");
    };

    //USE_MQTT = char(EEPROM.read(300));

    Serial.print("connected! IP address: ");
    Serial.println(WiFi.localIP());

    ArduinoOTA.onStart([]() {
      Serial.println("OTA Start");
    });
    
    ArduinoOTA.onEnd([]() {
      Serial.println("\nEnd");  //  "Завершение OTA-апдейта"
    });
    
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
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
  //EEPROM.begin(202);
  
  delay(50);
  if (EEPROM.read(198) != 20) {   // первый запуск
    EEPROM.write(198, 20);
    EEPROM.commit();

    for (byte i = 0; i < MODE_AMOUNT; i++) {
      EEPROM.put(3 * i + 40, modes[i]);
      EEPROM.commit();
    }
    for (byte i = 0; i < 7; i++) {
      EEPROM.write(5 * i, alarm[i].state);   // рассвет
      eeWriteInt(5 * i + 1, alarm[i].time);
      EEPROM.commit();
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
  
}

void MQTTcallback(char* topic, byte* payload, unsigned int length) {
  String Payload = "";

  #ifdef DEBUG
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  #endif

  for (int i = 0; i < length; i++) Payload += (char)payload[i];
  #ifdef DEBUG
  Serial.println(Payload);
  #endif

  if (String(topic) == "homeassistant/light/"+clientId+"/switch") {
      #ifdef DEBUG
      Serial.print("Command arrived: ");
      Serial.println(Payload);
      #endif

      if (Payload == "ON")  {
        ONflag = true;
        changePower();
        sendCurrent();
        mqttclient.publish(String("homeassistant/light/"+clientId+"/status").c_str(), "ON");
      }
      
      if (Payload == "OFF") {
        ONflag = false;
        changePower();
        sendCurrent();
        mqttclient.publish(String("homeassistant/light/"+clientId+"/status").c_str(), "OFF");
      }
  }

  if (String(topic) == "homeassistant/light/"+clientId+"/brightness/set") {
      #ifdef DEBUG
      Serial.print("Command arrived: brightness "); Serial.println(Payload);
      #endif

      modes[currentMode].brightness = Payload.toInt();
      FastLED.setBrightness(modes[currentMode].brightness);
      settChanged = true;
      eepromTimer = millis();
      mqttclient.publish(String("homeassistant/light/"+clientId+"/brightness/status").c_str(), String(modes[currentMode].brightness).c_str());

  }

  if (String(topic) == "homeassistant/light/"+clientId+"/effect/set") {
      #ifdef DEBUG
      Serial.print("Command arrived: effect set to "); Serial.println(Payload);
      #endif

      saveEEPROM();

      if (Payload.equals("Конфетти")) currentMode = 0;
      if (Payload.equals("Огонь")) currentMode = 1;
      if (Payload.equals("Радуга верт.")) currentMode = 2;
      if (Payload.equals("Радуга гориз.")) currentMode = 3;
      if (Payload.equals("Смена цвета")) currentMode = 4;
      if (Payload.equals("Безумие 3D")) currentMode = 5;
      if (Payload.equals("Облака 3D")) currentMode = 6;
      if (Payload.equals("Лава 3D")) currentMode = 7;
      if (Payload.equals("Плазма 3D")) currentMode = 8;
      if (Payload.equals("Радуга 3D")) currentMode = 9;
      if (Payload.equals("Павлин 3D")) currentMode = 10;
      if (Payload.equals("Зебра 3D")) currentMode = 11;
      if (Payload.equals("Лес 3D")) currentMode = 12;
      if (Payload.equals("Океан 3D")) currentMode = 13;
      if (Payload.equals("Цвет")) currentMode = 14;
      if (Payload.equals("Снегопад")) currentMode = 15;
      if (Payload.equals("Матрица")) currentMode = 16;
      if (Payload.equals("Светлячки")) currentMode = 17;

      loadingFlag = true;
      FastLED.clear();
      delay(1);
      sendCurrent();
      FastLED.setBrightness(modes[currentMode].brightness);
      
      mqttclient.publish(String("homeassistant/light/"+clientId+"/effect/status").c_str(), Payload.c_str());

  }

  if (String(topic) == "homeassistant/light/"+clientId+"/rgb/set") {
      #ifdef DEBUG
      Serial.print("Command arrived: rgb "); Serial.println(Payload);
      #endif
        
      int r = getValue(Payload, ',', 0).toInt();
      int g = getValue(Payload, ',', 1).toInt();
      int b = getValue(Payload, ',', 2).toInt();

      FastLED.clear();

      for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CRGB(r, g, b);
      }

      currentMode = 14;
      loadingFlag = true;
      delay(1);
      sendCurrent();
      FastLED.setBrightness(modes[currentMode].brightness);

      mqttclient.publish(String("homeassistant/light/"+clientId+"/rgb/status").c_str(), Payload.c_str());
      mqttclient.publish(String("homeassistant/light/"+clientId+"/effect/status").c_str(), "Цвет");

  }

  if (String(topic) == "homeassistant/light/"+clientId+"/effect/speed/set") {
      #ifdef DEBUG
      Serial.print("Command arrived: speed "); Serial.println(Payload);
      #endif

      modes[currentMode].speed = Payload.toInt();
      loadingFlag = true;
      settChanged = true;
      eepromTimer = millis();

      mqttclient.publish(String("homeassistant/light/"+clientId+"/effect/speed/status").c_str(), Payload.c_str());

  }

  if (String(topic) == "homeassistant/light/"+clientId+"/effect/scale/set") {
      #ifdef DEBUG
      Serial.print("Command arrived: scale "); Serial.println(Payload);
      #endif

      modes[currentMode].scale = Payload.toInt();
      loadingFlag = true;
      settChanged = true;
      eepromTimer = millis();

      mqttclient.publish(String("homeassistant/light/"+clientId+"/effect/scale/status").c_str(), Payload.c_str());

  }

}

unsigned long timing = 0;

void MQTTreconnect() {
  if (USE_MQTT && (millis() - timing > 5000)) {

      MQTTconfig MQTTConfig = readMQTTConfig();

      mqttclient.setServer(MQTTConfig.HOST, 1883);
      mqttclient.setCallback(MQTTcallback);

      if ((millis() - timing > 5000) && !mqttclient.connected()) {
        timing = millis();
        Serial.print("Attempting MQTT connection...");

        // подключаемся к MQTT серверу
          if (mqttclient.connect(clientId.c_str(), MQTTConfig.USER, MQTTConfig.PASSWD)) {
            Serial.println("connected!");

            #ifdef DEBUG
            mqttclient.subscribe(String("homeassistant/light/"+clientId+"/config").c_str());
            #endif

            HomeAssistantSendDiscoverConfig();

            // подписываемся на топики      
            mqttclient.subscribe(String("homeassistant/light/"+clientId+"/switch").c_str());
            mqttclient.subscribe(String("homeassistant/light/"+clientId+"/status").c_str());

            mqttclient.subscribe(String("homeassistant/light/"+clientId+"/brightness/status").c_str());
            mqttclient.subscribe(String("homeassistant/light/"+clientId+"/brightness/set").c_str());

            mqttclient.subscribe(String("homeassistant/light/"+clientId+"/effect/status").c_str());
            mqttclient.subscribe(String("homeassistant/light/"+clientId+"/effect/set").c_str());

            mqttclient.subscribe(String("homeassistant/light/"+clientId+"/rgb/status").c_str());
            mqttclient.subscribe(String("homeassistant/light/"+clientId+"/rgb/set").c_str());

            mqttclient.subscribe(String("homeassistant/light/"+clientId+"/effect/speed/status").c_str());
            mqttclient.subscribe(String("homeassistant/light/"+clientId+"/effect/speed/set").c_str());
          
            mqttclient.subscribe(String("homeassistant/light/"+clientId+"/effect/scale/status").c_str());
            mqttclient.subscribe(String("homeassistant/light/"+clientId+"/effect/scale/set").c_str());
          
        } else {
  
          Serial.print("failed, rc=");
          Serial.print(mqttclient.state());
          Serial.println(" try again in 5 seconds");
        }
      }
    }
  }

void HomeAssistantSendDiscoverConfig() {

  DynamicJsonDocument hass_discover(2048);
  
  hass_discover["name"] = "Gyver Lamp "+ clientId; // name
  hass_discover["uniq_id"] = String(ESP.getChipId(), HEX); // unique_id

  hass_discover["bri_cmd_t"] = "homeassistant/light/"+clientId+"/brightness/set";     // brightness_command_topic
  hass_discover["bri_stat_t"] = "homeassistant/light/"+clientId+"/brightness/status"; // brightness_state_topic
  hass_discover["bri_scl"] = 100;

  hass_discover["cmd_t"] = "homeassistant/light/"+clientId+"/switch"; // command_topic
  hass_discover["stat_t"] = "homeassistant/light/"+clientId+"/status"; // state_topic
  
  hass_discover["fx_cmd_t"] = "homeassistant/light/"+clientId+"/effect/set";     // effect_command_topic
  hass_discover["fx_stat_t"] = "homeassistant/light/"+clientId+"/effect/status"; // effect_state_topic

  hass_discover["rgb_cmd_t"] = "homeassistant/light/"+clientId+"/rgb/set";     // rgb_command_topic
  hass_discover["rgb_stat_t"] = "homeassistant/light/"+clientId+"/rgb/status"; // rgb_state_topic

  String hass_discover_str;
  serializeJson(hass_discover, hass_discover_str);

  const char eff_list[] = R"=====(, "fx_list": ["Конфетти", "Огонь", "Радуга верт.", "Радуга гориз.", "Смена цвета", "Безумие 3D", "Облака 3D", "Лава 3D", "Плазма 3D", "Радуга 3D", "Павлин 3D", "Зебра 3D", "Лес 3D", "Океан 3D", "Цвет", "Снегопад", "Матрица", "Светлячки"] })=====";  // effect_list

  hass_discover_str = hass_discover_str.substring(0, hass_discover_str.length() - 1);
  hass_discover_str += eff_list;

  #ifdef DEBUG
  Serial.println(hass_discover_str);
  mqttclient.publish(String("homeassistant/light/"+clientId+"/config").c_str(), "");
  #endif

  mqttclient.publish(String("homeassistant/light/"+clientId+"/config").c_str(), hass_discover_str.c_str()) ? Serial.println("Success sent discover message") : Serial.println("Error sending discover message");
}

void loop() {
  parseUDP();
  effectsTick();
  eepromTick();
  timeTick();
  buttonTick();

  if (USE_MQTT && !mqttclient.connected()) MQTTreconnect();
  if (USE_MQTT) mqttclient.loop();

  ArduinoOTA.handle();
  
  ESP.wdtFeed();   // пнуть собаку
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

String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length()-1;

  for(int i=0; i<=maxIndex && found<=index; i++){
    if(data.charAt(i)==separator || i==maxIndex){
        found++;
        strIndex[0] = strIndex[1]+1;
        strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }

  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}
