CHSV dawnColor;

/*
    timeset - callback вызывается при установке времени
*/
void timeset(){
  timeavailable =1;
}

void timeTick() {
  if (! timeavailable) return;  // nothing to do if time has not been set yet

  if (millis()/1000 % 3 == 0) {   // каждые 3 секунды проверяем рассвет
    now = time(nullptr);
    checkDawn(localtime(&now));
  }
  if (dawnFlag && timeStrTimer.isReady()) {
    fill_solid(leds, NUM_LEDS, dawnColor);
    fillString(gettimeStr(), CRGB::Black, false);
    delay(1);
    yield();
    FastLED.show();
  }
}

String gettimeStr() {
    char timeStr[6];
    now = time(nullptr);
    sprintf(timeStr, "%u:%02u", localtime(&now)->tm_hour, localtime(&now)->tm_min);
    return String(timeStr);
}

void checkDawn(const tm* tm) {
  byte thisDay = tm->tm_wday;
  if (thisDay == 0) thisDay = 7;  // воскресенье это 0
  thisDay--;
  thisTime = tm->tm_hour * 60 + tm->tm_min + (float)tm->tm_sec / 60;

  // проверка рассвета
  if (alarm[thisDay].state &&                                       // день будильника
      thisTime >= (alarm[thisDay].time - dawnOffsets[dawnMode]) &&  // позже начала
      thisTime < (alarm[thisDay].time + DAWN_TIMEOUT) ) {           // раньше конца + минута
    if (!manualOff) {
      // величина рассвета 0-255
      int dawnPosition = (float)255 * ((float)((float)thisTime - (alarm[thisDay].time - dawnOffsets[dawnMode])) / dawnOffsets[dawnMode]);
      dawnPosition = constrain(dawnPosition, 0, 255);
      dawnColor = CHSV(map(dawnPosition, 0, 255, 10, 35),
                       map(dawnPosition, 0, 255, 255, 170),
                       map(dawnPosition, 0, 255, 10, DAWN_BRIGHT));
      FastLED.setBrightness(255);
      dawnFlag = true;
    }
  } else {
    if (dawnFlag) {
      dawnFlag = false;
      manualOff = false;
      FastLED.setBrightness(modes[currentMode].brightness);
      FastLED.clear();
      FastLED.show();
    }
  }
}
