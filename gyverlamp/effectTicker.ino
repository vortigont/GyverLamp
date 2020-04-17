#include "effectsTicker.h"

/*
 * lampfade - lamp power fade-in, fade-out call-back
 * incrementaly fades lamp brightness
 * @param bool fade - fade 1 -in, 0 -out
 *
 */
class FaderSingleTone {
  // private
  CFastLED _led;
  uint8_t _steps, _maxbrt;
  uint32_t _timeout;
  bool _fade;
  FaderSingleTone* pInstance = nullptr;
  Ticker _fadeTicker;

  FaderSingleTone(CFastLED &led, bool fade, uint8_t brightness, uint32_t timeout=FADE_TIMEOUT) :
      _led(led), _fade(fade), _steps(brightness), _timeout(timeout) {
      if ( _fade ) _maxbrt = _steps;
      attach(round(_timeout/_steps));
  }

public:
  ~FaderSingleTone() {;};

  void fader() {
      --pInstance->_steps;
      _led->setBrightness(_fade ? _maxbrt - _steps : _steps);
      _led->show();
      if ( pInstance->_steps == 0 ) {
          detach();
      }
  }

  bool attach(uint32_t period) {
      _fadeTicker.attach_ms(period, std::bind(&FaderSingleTone::fader, this));
      return _fadeTicker.active();
  }

  bool detach() {
      if (_fadeTicker.active()) {
          _fadeTicker.detach();
      }
      return _fadeTicker.active();
  }

  const inline bool isAttached() const { return _fadeTicker.active(); };

  static FaderSingleTone& getInstance() {
        return &pInstance;
  }

  static FaderSingleTone& getInstance(CFastLED &led, bool fade, uint8_t brightness, uint32_t timeout=FADE_TIMEOUT) {
    if (pInstance == nullptr) {
        pInstance = new FaderSingleTone(led, fade, brightness, timeout=FADE_TIMEOUT);
    }
    return &pInstance;
  }
};

/*
  *  effectGettimer - calculates timer value for particular effect
  * @param uint8_t id - effect number id out of all available confgurations
  * @return uint32_t effect update rate in ms
 */
uint32_t effectGetUpdRate(uint8_t id){
  return (id < 5 || id > 13) ? modes[id].speed : SPEED_NOISE;
}


void effectsTick() {
  if (dawnFlag ) return;

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

void changePower() {
  if (ONflag && ! tickerEffects.active()) return;

  if (ONflag) {
    // Включение
    //stop_eff = false;
      tickerEffects.attach_ms_scheduled(effectGetUpdRate(currentMode), effectsTick);
  } else {
    // Выключение
    tickerTrigger.once_ms_scheduled(FADE_TIMEOUT, powerdown);
  }

  FaderSingleTone::getInstance().attach(FastLED, ONflag, modes[currentMode].brightness, FADE_TIMEOUT);

}

void powerdown() {
  tickerEffects.detach();
  FastLED.clear();
  FastLED.show();
  // записываем статус лампы в память
  EEPROM.write(420, ONflag); EEPROM.commit();
}
