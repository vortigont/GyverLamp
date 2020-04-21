/*
  Not sure why app accepts two types of messages for DEB replies:
  CURR - with current lamp configuration
  OK - timestamp heartbeats
  since udp requests are unidirectional, we have no way to upate app about CFG changes except
  that feed it with CURR reply instead of OK for DEB messages. For that we need to track every config change
  via mqtt/button/alarm clock. Since there are class wrappings for now to make it confortabley
  I will feed app with a round-robbin mix of hearbeats and configs, hope it will make app happy
  both for recent config and connection status.
  Also sometimes app sends back config updates with the same settings just received from lamp,
  this produces unnnecesary eeprom/mqtt updates and should be discarded
*/

bool rr = true;   // round-robbin flag for config/heartbeat replies

void parseUDP() {
  bool update = true;
  if (! Udp.parsePacket() ) return;

    int n = Udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
    packetBuffer[n] = 0;
    inputBuffer = packetBuffer;

    if (inputBuffer.startsWith("DEB")) {
      update = false;
      if (rr) {
        now = time(nullptr);
        char buf[11];
        sprintf(buf, "OK %02u:%02u:%02u",localtime(&now)->tm_hour, localtime(&now)->tm_min, localtime(&now)->tm_sec);
        inputBuffer = String(buf);
      } else sendCurrent();
      rr = !rr;
    } else if (inputBuffer.startsWith("GET")) {
      update = false;
      sendCurrent();
    } else if (inputBuffer.startsWith("EFF")) {
      if (currentMode == (byte)inputBuffer.substring(3).toInt()) return;
      currentMode = (byte)inputBuffer.substring(3).toInt();
      loadingFlag = true;
      FastLED.clear();
      delay(1);
      lamp.setBrightness(modes[currentMode].brightness);
    } else if (inputBuffer.startsWith("BRI")) {
      if (modes[currentMode].brightness == inputBuffer.substring(3).toInt()) return;
      modes[currentMode].brightness = inputBuffer.substring(3).toInt();
      lamp.setBrightness(modes[currentMode].brightness);
    } else if (inputBuffer.startsWith("SPD")) {
      if (modes[currentMode].speed == inputBuffer.substring(3).toInt()) return;
      modes[currentMode].speed = inputBuffer.substring(3).toInt();
      loadingFlag = true;
      tickerEffects.attach_ms_scheduled(effectGetUpdRate(currentMode), effectsTick);
    } else if (inputBuffer.startsWith("SCA")) {
      if (modes[currentMode].scale = inputBuffer.substring(3).toInt()) return;
      modes[currentMode].scale = inputBuffer.substring(3).toInt();
      loadingFlag = true;
    } else if (inputBuffer.startsWith("P_ON")) {
      ONflag = true;
      tickerHelper.once_ms_scheduled(0, std::bind(changePower,ONflag));
    } else if (inputBuffer.startsWith("P_OFF")) {
      ONflag = false;
      tickerHelper.once_ms_scheduled(0, std::bind(changePower,ONflag));
    } else if (inputBuffer.startsWith("ALM_SET")) {
      byte alarmNum = (char)inputBuffer[7] - '0';
      alarmNum -= 1;
      if (inputBuffer.indexOf("ON") != -1) {
        alarm[alarmNum].state = true;
        inputBuffer = "alm #" + String(alarmNum + 1) + " ON";
      } else if (inputBuffer.indexOf("OFF") != -1) {
        alarm[alarmNum].state = false;
        inputBuffer = "alm #" + String(alarmNum + 1) + " OFF";
      } else {
        int almTime = inputBuffer.substring(8).toInt();
        alarm[alarmNum].time = almTime;
        byte hour = floor(almTime / 60);
        byte minute = almTime - hour * 60;
        inputBuffer = "alm #" + String(alarmNum + 1) +
                      " " + String(hour) +
                      ":" + String(minute);
      }
      saveAlarm(alarmNum);
      manualOff = false;
      update = false;
      tickerAlarm.once_scheduled(0,  checkDawn);  // trigger Dawn checker
    } else if (inputBuffer.startsWith("ALM_GET")) {
      sendAlarms();
      update = false;
    } else if (inputBuffer.startsWith("DAWN")) {
      dawnMode = inputBuffer.substring(4).toInt() - 1;
      saveDawnMmode();
      update = false;
    }

    if ( update ) {
      settChanged = true;
      eepromTimer = millis();
      sendCurrent();
      tickerMQTT.attach_ms_scheduled(0, MQTTUpdateState);
    }
    sendSettings();
}

void sendCurrent() {
  inputBuffer = "CURR";
  inputBuffer += " ";
  inputBuffer += String(currentMode);
  inputBuffer += " ";
  inputBuffer += String(modes[currentMode].brightness);
  inputBuffer += " ";
  inputBuffer += String(modes[currentMode].speed);
  inputBuffer += " ";
  inputBuffer += String(modes[currentMode].scale);
  inputBuffer += " ";
  inputBuffer += String(ONflag);
}

void sendSettings() {
  char reply[inputBuffer.length() + 1];
  inputBuffer.toCharArray(reply, inputBuffer.length() + 1);
  Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
  Udp.write(reply);
  Udp.endPacket();
}

void sendAlarms() {
  inputBuffer = "ALMS ";
  for (byte i = 0; i < 7; i++) {
    inputBuffer += String(alarm[i].state);
    inputBuffer += " ";
  }
  for (byte i = 0; i < 7; i++) {
    inputBuffer += String(alarm[i].time);
    inputBuffer += " ";
  }
  inputBuffer += (dawnMode + 1);
}
