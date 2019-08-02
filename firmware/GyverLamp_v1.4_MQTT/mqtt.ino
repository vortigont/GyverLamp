
MQTTconfig readMQTTConfig () {
  int eeAddress = 300;
  MQTTconfig MQTTConfig;

  for (int i = 0; i < 32; ++i) MQTTConfig.HOST[i]   = char(EEPROM.read(eeAddress + i)); eeAddress += 32;
  for (int i = 0; i < 32; ++i) MQTTConfig.USER[i]   = char(EEPROM.read(eeAddress + i)); eeAddress += 32;
  for (int i = 0; i < 32; ++i) MQTTConfig.PASSWD[i] = char(EEPROM.read(eeAddress + i)); 

  return MQTTConfig;
}

void writeMQTTConfig(const char HOST[32], const char USER[32], const char PASSWD[32]) {
  int eeAddress = 300;

  for (int i = 0; i < 32; ++i)  EEPROM.write(eeAddress + i, HOST[i]); eeAddress += 32;
  for (int i = 0; i < 32; ++i)  EEPROM.write(eeAddress + i, USER[i]); eeAddress += 32;
  for (int i = 0; i < 32; ++i)  EEPROM.write(eeAddress + i, PASSWD[i]); 

  EEPROM.commit();
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

      modes[currentMode].speed = 255 - Payload.toInt();
      loadingFlag = true;
      settChanged = true;
      eepromTimer = millis();

      mqttclient.publish(String("homeassistant/light/"+clientId+"/effect/speed/status").c_str(), Payload.c_str());

  }

  if (String(topic) == "homeassistant/light/"+clientId+"/effect/scale/set") {
      #ifdef DEBUG
      Serial.print("Command arrived: scale "); Serial.println(Payload);
      #endif

      if (currentMode == 17 && Payload.toInt() > 100) Payload = "100";
      
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

  mqttclient.publish(String("homeassistant/light/"+clientId+"/config").c_str(), hass_discover_str.c_str(), true) ? Serial.println("Success sent discover message") : Serial.println("Error sending discover message");
}
