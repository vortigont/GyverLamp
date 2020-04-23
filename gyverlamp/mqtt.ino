#include "mqtt.h"

uint8_t mqtt_reconnection_count = 0;

MQTTconfig readMQTTConfig () {
  int eeAddress = 300;
  MQTTconfig MQTTConfig;

  for (int i = 0; i < 32; ++i) MQTTConfig.HOST[i]   = char(EEPROM.read(eeAddress + i)); eeAddress += 32;
  for (int i = 0; i < 32; ++i) MQTTConfig.USER[i]   = char(EEPROM.read(eeAddress + i)); eeAddress += 32;
  for (int i = 0; i < 32; ++i) MQTTConfig.PASSWD[i] = char(EEPROM.read(eeAddress + i)); eeAddress += 32;
  for (int i = 0; i < 10; ++i) MQTTConfig.PORT[i] = char(EEPROM.read(eeAddress + i));

  return MQTTConfig;
}

void writeMQTTConfig(const char HOST[32], const char USER[32], const char PASSWD[32], const char PORT[10]) {
  int eeAddress = 300;

  for (int i = 0; i < 32; ++i)  EEPROM.write(eeAddress + i, HOST[i]); eeAddress += 32;
  for (int i = 0; i < 32; ++i)  EEPROM.write(eeAddress + i, USER[i]); eeAddress += 32;
  for (int i = 0; i < 32; ++i)  EEPROM.write(eeAddress + i, PASSWD[i]); eeAddress += 32;
  for (int i = 0; i < 10; ++i)  EEPROM.write(eeAddress + i, PORT[i]);

  EEPROM.commit();
}

int Get_EFFIDX (String effect) {

  if (effect.equals("Конфетти")) return 0;
  if (effect.equals("Огонь")) return 1;
  if (effect.equals("Радуга верт.")) return 2;
  if (effect.equals("Радуга гориз.")) return 3;
  if (effect.equals("Смена цвета")) return 4;
  if (effect.equals("Безумие 3D")) return 5;
  if (effect.equals("Облака 3D")) return 6;
  if (effect.equals("Лава 3D")) return 7;
  if (effect.equals("Плазма 3D")) return 8;
  if (effect.equals("Радуга 3D")) return 9;
  if (effect.equals("Павлин 3D")) return 10;
  if (effect.equals("Зебра 3D")) return 11;
  if (effect.equals("Лес 3D")) return 12;
  if (effect.equals("Океан 3D")) return 13;
  if (effect.equals("Цвет")) return 14;
  if (effect.equals("Снегопад")) return 15;
  if (effect.equals("Матрица")) return 16;
  if (effect.equals("Светлячки")) return 17;
  if (effect.equals("Аквариум")) return 18;
  if (effect.equals("Звездопад")) return 19;
  if (effect.equals("Пейнтбол")) return 20;
  if (effect.equals("Спираль")) return 21;
  if (effect.equals("Демо")) return 22;

}

String Get_EFFName (int eff_idx) {

  switch (eff_idx) {
    case 0: return "Конфетти";
    case 1: return "Огонь";
    case 2: return "Радуга верт.";
    case 3: return "Радуга гориз.";
    case 4: return "Смена цвета";
    case 5: return "Безумие 3D";
    case 6: return "Облака 3D";
    case 7: return "Лава 3D";
    case 8: return "Плазма 3D";
    case 9: return "Радуга 3D";
    case 10: return "Павлин 3D";
    case 11: return "Зебра 3D";
    case 12: return "Лес 3D";
    case 13: return "Океан 3D";
    case 14: return "Цвет";
    case 15: return "Снегопад";
    case 16: return "Матрица";
    case 17: return "Светлячки";
    case 18: return "Аквариум";
    case 19: return "Звездопад";
    case 20: return "Пейнтбол";
    case 21: return "Спираль";
    case 22: return "Демо";
  }

}

void MQTTUpdateState () {
   if (! USE_MQTT) return;
   if (! mqttclient.connected()) { tickerMQTT.once_scheduled(0,MQTTreconnect); return;}

   _SPLN("MQTT Update state");
   mqttclient.publish(String("homeassistant/light/"+clientId+"/status").c_str(), ONflag ? "ON" : "OFF", true);
   mqttclient.publish(String("homeassistant/light/"+clientId+"/brightness/status").c_str(), String(modes[currentMode].brightness).c_str(), true);
   mqttclient.publish(String("homeassistant/light/"+clientId+"/effect/status").c_str(), Get_EFFName(currentMode).c_str(), true);
   mqttclient.publish(String("homeassistant/light/"+clientId+"/effect/speed/status").c_str(), String(modes[currentMode].speed).c_str(), true);
   mqttclient.publish(String("homeassistant/light/"+clientId+"/effect/scale/status").c_str(), String(modes[currentMode].scale).c_str(), true);

   char sRGB[15];
   sprintf(sRGB, "%d,%d,%d", r, g, b);
   mqttclient.publish(String("homeassistant/light/"+clientId+"/rgb/status").c_str(), sRGB, true);
   tickerMQTT.attach_scheduled(TIMER_MQTT, infoCallback);
}

void MQTTcallback(char* topic, byte* payload, unsigned int length) {
  String Payload = "";

  _SP("Message arrived [");
  _SP(topic);
  _SP("] ");

  for (int i = 0; i < length; i++) Payload += (char)payload[i];
  _SPLN(Payload);

  if (String(topic) == "homeassistant/light/"+clientId+"/switch") {

      _SP("Command arrived: ");
      _SPLN(Payload);

      ONflag = (Payload == "ON") ? true : false;
      changePower(ONflag);
      MQTTUpdateState();
  }

  if (String(topic) == "homeassistant/light/"+clientId+"/brightness/set") {

      _SP("Command arrived: brightness "); _SPLN(Payload);

      modes[currentMode].brightness = Payload.toInt();
      lamp.setBrightness(modes[currentMode].brightness);
      settChanged = true;
      eepromTimer = millis();
      MQTTUpdateState();

  }

  if (String(topic) == "homeassistant/light/"+clientId+"/effect/set") {

      _SP("Command arrived: effect set to "); _SPLN(Payload);

      if (Payload == "Демо") {

          demoCallback();
          tickerDemo.attach_scheduled(TIMER_DEMO, demoCallback);
      } else {
          tickerDemo.detach();
          currentMode = Get_EFFIDX(Payload);
       }

      saveEEPROM();
      loadingFlag = true;
      FastLED.clear();
      delay(1);
      sendCurrent();
      lamp.setBrightness(modes[currentMode].brightness);
      MQTTUpdateState();

  }

  if (String(topic) == "homeassistant/light/"+clientId+"/rgb/set") {

      _SP("Command arrived: rgb "); _SPLN(Payload);

      r = getValue(Payload, ',', 0).toInt();
      g = getValue(Payload, ',', 1).toInt();
      b = getValue(Payload, ',', 2).toInt();

      currentMode = 14;
      loadingFlag = true;
      delay(1);
      sendCurrent();
      lamp.setBrightness(modes[currentMode].brightness);
      MQTTUpdateState();

  }

  if (String(topic) == "homeassistant/light/"+clientId+"/effect/speed/set") {

      _SP("Command arrived: speed "); _SPLN(Payload);

      modes[currentMode].speed = Payload.toInt();
      saveEEPROM();
      loadingFlag = true;
      settChanged = true;
      eepromTimer = millis();
      MQTTUpdateState();
      tickerEffects.attach_ms_scheduled(effectGetUpdRate(currentMode), effectsTick);
  }

  if (String(topic) == "homeassistant/light/"+clientId+"/effect/scale/set") {

      _SP("Command arrived: scale "); _SPLN(Payload);

      if (currentMode == 17 && Payload.toInt() > 100) Payload = "100";

      modes[currentMode].scale = Payload.toInt();
      saveEEPROM();
      loadingFlag = true;
      settChanged = true;
      eepromTimer = millis();
      MQTTUpdateState();

  }

}


void MQTTreconnect() {
      MQTTconfig MQTTConfig = readMQTTConfig();

      mqttclient.setServer(MQTTConfig.HOST, String(MQTTConfig.PORT).toInt());
      mqttclient.setCallback(MQTTcallback);

      _SPTO(Serial.printf_P(dbg_conattempt_P, MQTTConfig.HOST, MQTTConfig.PORT, MQTTConfig.USER));

      // подключаемся к MQTT серверу
      if (mqttclient.connect(clientId.c_str(), MQTTConfig.USER, MQTTConfig.PASSWD, String("homeassistant/light/"+clientId+"/state").c_str(), 0,  true, "online")) {
            _SPLN("connected!");
            mqtt_reconnection_count = 0;

            // подписываемся на топики
            mqttclient.subscribe(String("homeassistant/light/"+clientId+"/switch").c_str());
            mqttclient.subscribe(String("homeassistant/light/"+clientId+"/brightness/set").c_str());
            mqttclient.subscribe(String("homeassistant/light/"+clientId+"/effect/set").c_str());
            mqttclient.subscribe(String("homeassistant/light/"+clientId+"/rgb/set").c_str());
            mqttclient.subscribe(String("homeassistant/light/"+clientId+"/effect/speed/set").c_str());
            mqttclient.subscribe(String("homeassistant/light/"+clientId+"/effect/scale/set").c_str());

            tickerMQTT.once_ms_scheduled(0, HomeAssistantSendDiscoverConfig);
      } else {

          if (mqtt_reconnection_count == MQTT_RECONNECT_ATTEMPTS) {
            mqtt_reconnection_count = 0;
          }

          ++mqtt_reconnection_count;

          _SP("failed, rc=");
          _SP(mqttclient.state());
          _SPF(" try again in %d seconds\n", 2<<mqtt_reconnection_count);
          tickerMQTT.once_scheduled(2 << mqtt_reconnection_count, MQTTreconnect);
      }
}

void HomeAssistantSendDiscoverConfig() {

  char espid[ESP_ID_LEN];
  sprintf(espid, "%x", ESP.getChipId());

  char topic[MQTT_TOPIC_BUFF_LEN];

  uint16_t msglength = 0;

  for (uint8_t i = 0; i < HA_DISCOVER_PGMCHUNKS; i++) {
    msglength += strlen_P((PGM_P)HA_discover_dev[i]);
  }
  
  msglength += strlen(espid) * HA_DISCOVER_IDCHUNKS;
  _SPTO(Serial.printf_P(dbg_publishlong_P, msglength));

  snprintf_P(topic, sizeof(topic), topic_light_id_config_P, clientId.c_str());

  if (mqttclient.beginPublish(topic, msglength, true) ) {
    mqttclient.print(FPSTR(HA_discover00));  _SP(FPSTR(HA_discover00));
    mqttclient.print(espid);                 _SP(espid);
    mqttclient.print(FPSTR(HA_discover01));  _SP(FPSTR(HA_discover01));
    mqttclient.print(espid);                 _SP(espid);
    mqttclient.print(FPSTR(HA_discover02));  _SP(FPSTR(HA_discover02));
    mqttclient.print(espid);                 _SP(espid);
    mqttclient.print(FPSTR(HA_discover03));  _SP(FPSTR(HA_discover03));
    mqttclient.print(FPSTR(HA_discover04));  _SP(FPSTR(HA_discover04));
    mqttclient.print(FPSTR(HA_discover05));  _SP(FPSTR(HA_discover05));
    mqttclient.print(espid);                 _SP(espid);
    mqttclient.print(FPSTR(HA_discover06));  _SP(FPSTR(HA_discover06));

    if ( not mqttclient.endPublish() ) _SPLN(FPSTR(dbg_publish_P));
  } else {
    _SPLN(FPSTR(dbg_publishcomplete_P));
  }


  // уровень wifi сигнала
  msglength = 0;
  for (uint8_t i = 0; i < HA_DISCOVER_S_PGMCHUNKS; i++) {
    msglength += strlen_P((PGM_P)HA_discover_sig[i]);
  }
  
  msglength += strlen(espid) * HA_DISCOVER_S_IDCHUNKS;
  _SPTO(Serial.printf_P(dbg_publishlong_P, msglength));

  snprintf_P(topic, sizeof(topic), topic_sensor_id_Wconfig_P, clientId.c_str());

  if (mqttclient.beginPublish(topic, msglength, true) ) {
    mqttclient.print(FPSTR(HA_discover_s00));   _SP(FPSTR(HA_discover_s00));
    mqttclient.print(espid);                    _SP(espid);
    mqttclient.print(FPSTR(HA_discover_s01));   _SP(FPSTR(HA_discover_s01));
    mqttclient.print(espid);                    _SP(espid);
    mqttclient.print(FPSTR(HA_discover_s02));   _SP(FPSTR(HA_discover_s02));
    mqttclient.print(espid);                    _SP(espid);
    mqttclient.print(FPSTR(HA_discover_s03));   _SP(FPSTR(HA_discover_s03));
    mqttclient.print(espid);                    _SP(espid);
    mqttclient.print(FPSTR(HA_discover_s04));   _SP(FPSTR(HA_discover_s04));
    mqttclient.print(espid);                    _SP(espid);
    mqttclient.print(FPSTR(HA_discover_s05));   _SP(FPSTR(HA_discover_s05));

    if ( not mqttclient.endPublish() ) _SPLN(FPSTR(dbg_publish_P));
  } else {
    _SPLN(FPSTR(dbg_publishcomplete_P));
  }

  // Время непрерывной работы
  msglength = 0;
  for (uint8_t i = 0; i < HA_DISCOVER_U_PGMCHUNKS; i++) {
    msglength += strlen_P((PGM_P)HA_discover_upt[i]);
  }
  
  msglength += strlen(espid) * HA_DISCOVER_U_IDCHUNKS;
  _SPTO(Serial.printf_P(dbg_publishlong_P, msglength));

  snprintf_P(topic, sizeof(topic), topic_sensor_id_Uconfig_P, clientId.c_str());

  if (mqttclient.beginPublish(topic, msglength, true) ) {
    mqttclient.print(FPSTR(HA_discover_s00));   _SP(FPSTR(HA_discover_s00));
    mqttclient.print(espid);                    _SP(espid);
    mqttclient.print(FPSTR(HA_discover_u01));   _SP(FPSTR(HA_discover_u01));
    mqttclient.print(espid);                    _SP(espid);
    mqttclient.print(FPSTR(HA_discover_u02));   _SP(FPSTR(HA_discover_u02));
    mqttclient.print(espid);                    _SP(espid);
    mqttclient.print(FPSTR(HA_discover_s03));   _SP(FPSTR(HA_discover_s03));
    mqttclient.print(espid);                    _SP(espid);
    mqttclient.print(FPSTR(HA_discover_s04));   _SP(FPSTR(HA_discover_s04));
    mqttclient.print(espid);                    _SP(espid);
    mqttclient.print(FPSTR(HA_discover_s05));   _SP(FPSTR(HA_discover_s05));

    if ( not mqttclient.endPublish() ) _SPLN(FPSTR(dbg_publish_P));
  } else {
    _SPLN(FPSTR(dbg_publishcomplete_P));
  }

  tickerMQTT.once_ms_scheduled(0, MQTTUpdateState);
}

void infoCallback() {
    if (! USE_MQTT) return;
    if (! mqttclient.connected()) { _SPLN("No MQTT connection"); tickerMQTT.once_scheduled(0,MQTTreconnect); return;}

    mqttclient.publish(String("homeassistant/sensor/"+clientId+"U/uptime").c_str(), String(millis()/1000).c_str(), true);
    mqttclient.publish(String("homeassistant/sensor/"+clientId+"W/WiFi/RSSI").c_str(), String(WiFi.RSSI()).c_str(), true);
    mqttclient.publish(String("homeassistant/sensor/"+clientId+"W/WiFi/RSSI_pct").c_str(), String(2*(WiFi.RSSI()+100)).c_str(), true);
    mqttclient.publish(String("homeassistant/sensor/"+clientId+"W/WiFi/channel").c_str(), String(WiFi.channel()).c_str(), true);
    mqttclient.publish(String("homeassistant/light/"+clientId+"/ResetReason").c_str(), String(ESP.getResetReason()).c_str(), true);
    mqttclient.publish(String("homeassistant/sensor/"+clientId+"/VCC").c_str(), String((float)ESP.getVcc()/1000.0f).c_str(), true);
    mqttclient.publish(String("homeassistant/light/"+clientId+"/BootCount").c_str(), String(boot_count).c_str(), true);
    mqttclient.publish(String("homeassistant/light/"+clientId+"/DemoMode").c_str(), String(tickerDemo.active()).c_str(), true);

    if (boot_count > 1) { boot_count = 0; EEPROM.write(410, boot_count); EEPROM.commit(); }
}
