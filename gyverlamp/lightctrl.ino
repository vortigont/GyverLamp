/*
  Скетч к проекту "Многофункциональный RGB светильник"
  Страница проекта (схемы, описания): https://alexgyver.ru/GyverLamp/
  Исходники на GitHub: https://github.com/AlexGyver/GyverLamp/
  Нравится, как написан код? Поддержи автора! https://alexgyver.ru/support_alex/
  Автор: AlexGyver, AlexGyver Technologies, 2019
  https://AlexGyver.ru/

  клон версии от whisler
  https://github.com/Whisler/GyverLamp

  code refactoring vortigont
  https://github.com/vortigont/GyverLamp

 *  This file        : lightctrl.ino
 *  This file Author : Emil Muratov
 *
 *  Description      : Light Controller class should maintain basic light functions, sush as:
 *                      - light on/off
 *                      - fader/brighness
 *                      - power status
 *                      - to be continued....
*/

#include "lightctrl.h"

LightCtrl::LightCtrl(){
    Ticker _fadeTicker;
    uint8_t _brt, _steps = 0;
    int8_t _brtincrement = 0;
}

LightCtrl::~LightCtrl(){}

void LightCtrl::fadelight(const uint8_t _targetbrightness, const uint32_t _duration) {
    _fadeTicker.detach();

    uint8_t _maxsteps = _duration / FADE_STEPTIME;
    _brt = getBrightness();
    uint8_t _brtdiff = abs(_targetbrightness - _brt);

    if (_brtdiff > FADE_MININCREMENT * _maxsteps) {
        _steps = _maxsteps;
    } else {
        _steps = _brtdiff/FADE_MININCREMENT;
    }

    if (_steps < 3) {
        brightness(_targetbrightness);
        return;
    }

    _brtincrement = (_targetbrightness - _brt) / _steps;

    _SPTO(Serial.printf_P(F_fadeinfo, _brt, _targetbrightness, _steps, _brtincrement)); _SPLN("");
    _fadeTicker.attach_ms(FADE_STEPTIME, std::bind(&LightCtrl::fader, this, _targetbrightness));
}

/*
 * Change global brightness with or without fade effect
 * fade applied in non-blocking way
 * FastLED dim8 function applied internaly for natural brightness controll
 * @param uint8_t _brt - target brigtness level 0-255
 * @param bool fade - use fade effect on brightness change
 */
void LightCtrl::setBrightness(const uint8_t _brt, const bool fade, const bool natural){
    _SP(F("Set brightness: ")); _SPLN(_brt);
    if (fade) {
        fadelight(_brt);
    } else {
        brightness(_brt, natural);
    }
}

/*
 * Get current brightness
 * FastLED brighten8 function applied internaly for natural brightness compensation
 * @param bool natural - return compensated or absolute brightness
 */
uint8_t LightCtrl::getBrightness(const bool natural){
    return (natural ? brighten8_raw(FastLED.getBrightness()) : FastLED.getBrightness());
}


/*
 * Set global brightness
 * @param bool natural 
 */
void LightCtrl::brightness(const uint8_t _brt, bool natural){
    uint8_t _cur = natural ? brighten8_raw(FastLED.getBrightness()) : FastLED.getBrightness();
    if ( _cur == _brt) return;

    FastLED.setBrightness(natural ? dim8_raw(_brt) : _brt);
    FastLED.show();
}

/*
 * Fade light callback
 * @param bool natural 
 */
void LightCtrl::fader(const uint8_t _tgtbrt){
  --_steps;
  if (! _steps) {   // on last step
      _fadeTicker.detach();
      _brt = _tgtbrt;
  } else {
      _brt += _brtincrement;
  }

  brightness(_brt);
}

