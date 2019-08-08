void timeTick() {
  if (ESP_MODE == 1) {    
    if (timeTimer.isReady()) {
      timeClient.update();
      byte thisDay = timeClient.getDay();
      if (thisDay == 0) thisDay = 7;  // воскресенье это 0
      thisDay--;
      thisTime = timeClient.getHours() * 60 + timeClient.getMinutes();

      // проверка рассвета
      if (alarm[thisDay].state &&                                       // день будильника
          thisTime >= (alarm[thisDay].time - dawnOffsets[dawnMode]) &&  // позже начала
          thisTime < (alarm[thisDay].time + DAWN_TIMEOUT) ) {                      // раньше конца + минута
        if (!manualOff) {
          // величина рассвета 0-255
          int dawnPosition = 255 * ((float)(thisTime - (alarm[thisDay].time - dawnOffsets[dawnMode])) / dawnOffsets[dawnMode]);
          dawnPosition = constrain(dawnPosition, 0, 255);
          CHSV dawnColor = CHSV(map(dawnPosition, 0, 255, 10, 35),
                                map(dawnPosition, 0, 255, 255, 170),
                                map(dawnPosition, 0, 255, 10, DAWN_BRIGHT));
          fill_solid(leds, NUM_LEDS, dawnColor);
          FastLED.setBrightness(255);
          FastLED.show();
          dawnFlag = true;
        }
      } else {
        if (dawnFlag) {
          dawnFlag = false;
          manualOff = false;
          FastLED.setBrightness(modes[currentMode].brightness);
        }
      }

    }
  }
}

String getTimeStampString() {
   time_t rawtime = timeClient.getEpochTime();
   struct tm * ti;
   ti = localtime (&rawtime);

   uint16_t year = ti->tm_year + 1900;
   String yearStr = String(year);

   uint8_t month = ti->tm_mon + 1;
   String monthStr = month < 10 ? "0" + String(month) : String(month);

   uint8_t day = ti->tm_mday;
   String dayStr = day < 10 ? "0" + String(day) : String(day);

   uint8_t hours = ti->tm_hour;
   String hoursStr = hours < 10 ? "0" + String(hours) : String(hours);

   uint8_t minutes = ti->tm_min;
   String minuteStr = minutes < 10 ? "0" + String(minutes) : String(minutes);

   uint8_t seconds = ti->tm_sec;
   String secondStr = seconds < 10 ? "0" + String(seconds) : String(seconds);

   return "Дата: "+ dayStr + "-" + monthStr + "-" + yearStr + ". " + "Время: " +
          hoursStr + ":" + minuteStr;
}
