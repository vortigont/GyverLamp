CHSV dawnColor;

/*
    timeset - callback вызывается при установке времени
*/
void timeset(){
  timeavailable =1;
}

void alarmTick() {
  now = time(nullptr);
  if (not checkDawn(localtime(&now)) ) return;

    fill_solid(leds, NUM_LEDS, dawnColor);
    fillString(gettimeStr(), CRGB::Black, false);
    FastLED.show();
}

String gettimeStr() {
    char timeStr[6];
    now = time(nullptr);
    sprintf(timeStr, "%u:%02u", localtime(&now)->tm_hour, localtime(&now)->tm_min);
    return String(timeStr);
}

bool checkDawn(const tm* tm) {
  byte thisDay = tm->tm_wday;
  if (thisDay == 0) thisDay = 7;  // воскресенье это 0
  thisDay--;
  uint16_t thisTime = tm->tm_hour * 60 + tm->tm_min;   // число минут с начала суток

  // проверка рассвета
  if (alarm[thisDay].state &&                                       // день будильника
      thisTime >= (alarm[thisDay].time - dawnOffsets[dawnMode]) &&  // позже начала
      thisTime < (alarm[thisDay].time + DAWN_TIMEOUT) ) {           // раньше конца + минута
      if (manualOff) return false;
      // секунд от начала рассвета
      int16_t dawnPosition = (thisTime - alarm[thisDay].time - dawnOffsets[dawnMode])*60 + tm->tm_sec;
      Serial.print("dawnPosition: "); Serial.println(dawnPosition);
      // вычисляем цвет рассвета на текущую секунду
      dawnColor = CHSV(map(dawnPosition, 1, 60*dawnOffsets[dawnMode], 10, 35),
                       map(dawnPosition, 1, 60*dawnOffsets[dawnMode], 255, 170),
                       map(dawnPosition, 1, 60*dawnOffsets[dawnMode], DAWN_MIN_BRIGHT, DAWN_MAX_BRIGHT));
      FastLED.setBrightness(255);
      dawnFlag = true;
  } else if (dawnFlag) {
      dawnFlag = false;
      manualOff = false;
      FastLED.setBrightness(modes[currentMode].brightness);
      FastLED.clear();
      FastLED.show();
  }
  return dawnFlag;
}
