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
    tickerEffects.attach_ms_scheduled(effectGetUpdRate(currentMode), effectsTick);  // start effects scheduler
    int steps = 1 + round(modes[currentMode].brightness / 80);
    for (int i = 0; i <= modes[currentMode].brightness; i += steps) {
      FastLED.setBrightness(i);
      delay(2);
      FastLED.show();
    }
    FastLED.setBrightness(modes[currentMode].brightness);
  } else {
    // Выключение
    int steps = 1 + round(modes[currentMode].brightness / 40);
    for (int i = modes[currentMode].brightness; i >= 0; i -= steps) {
      FastLED.setBrightness(i);
      delay(2);
      FastLED.show();
    }

    tickerEffects.detach();   // stop effects scheduler
    tickerAlarm.once_ms_scheduled(0,  checkDawn);  // start Dawn checker scheduler while lamp is off
    tickerHelper.once_ms_scheduled(effectGetUpdRate(currentMode), [](){    FastLED.clear(); FastLED.show();});
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
      FastLED.setBrightness(modes[currentMode].brightness);
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
