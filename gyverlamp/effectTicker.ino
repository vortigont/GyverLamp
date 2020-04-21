#include "effectsTicker.h"



void effectsTick() {
  if (dawnFlag) return;

  switch (currentMode) {
        case 0: sparklesRoutine();
          break;
        case 1: fireRoutine();
          break;
        case 2: rainbowVertical();
          break;
        case 3: rainbowHorizontal();
          break;
        case 4: colorsRoutine();
          break;
        case 5: madnessNoise();
          break;
        case 6: cloudNoise();
          break;
        case 7: lavaNoise();
          break;
        case 8: plasmaNoise();
          break;
        case 9: rainbowNoise();
          break;
        case 10: rainbowStripeNoise();
          break;
        case 11: zebraNoise();
          break;
        case 12: forestNoise();
          break;
        case 13: oceanNoise();
          break;
        case 14: colorRoutine();
          break;
        case 15: snowRoutine();
          break;
        case 16: matrixRoutine();
          break;
        case 17: lightersRoutine();
          break;
        case 18: aquaRoutine();
          break;
        case 19: starfallRoutine();
          break;
        case 20: lightBallsRoutine();
          break;
        case 21: spiroRoutine();
          break;
  }
  FastLED.show();
}


void changePower(bool power) {

  if (power) {
    // Включение
    lamp.fadelight(modes[currentMode].brightness);
    tickerEffects.attach_ms_scheduled(effectGetUpdRate(currentMode), effectsTick);  // start effects scheduler
  } else {
    // Выключение
    lamp.fadelight(0);
    tickerHelper.once_ms_scheduled(FADE_TIME, [](){ tickerEffects.detach();   FastLED.clear(true); tickerAlarm.once_ms_scheduled(0,  checkDawn); });
  }

  // записываем статус лампы в память
  EEPROM.write(420, power); EEPROM.commit();
}


void demoCallback() {

      currentMode = random(0, MODE_AMOUNT-1);
      if (!epilepsy) {
          while (currentMode == 4) currentMode = random(0, MODE_AMOUNT-1);
        }

      loadingFlag = true;
      FastLED.clear();
      lamp.setBrightness(modes[currentMode].brightness);
      MQTTUpdateState();
}

/*
  *  effectGettimer - calculates timer value for particular effect
  * @param uint8_t id - effect number id out of all available confgurations
  * @return uint32_t effect update rate in ms
 */
uint32_t effectGetUpdRate(uint8_t id){
  return (id < 5 || id > 13) ? SPEED_MIN - modes[id].speed : SPEED_NOISE;
}
