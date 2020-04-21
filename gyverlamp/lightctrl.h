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

#pragma once

#define FADE              true      // fade by default on brightness change
#define FADE_STEPTIME      100      // default time between fade steps, ms (2 seconds with max steps)
#define FADE_TIME         2000      // Default fade time, ms
#define FADE_MININCREMENT    3      // Minimal increment for brightness fade

// flash strings
_SPTO(static const char F_fadeinfo[] PROGMEM = "Fade from %u to %u in %i steps with increment %i");

class LightCtrl {
public:
    LightCtrl();
    ~LightCtrl();

    /*
     * Change global brightness with or without fade effect
     * fade applied in non-blocking way
     * FastLED dim8 function applied internaly for natural brightness controll
     * @param uint8_t _tgtbrt - target brigtness level 0-255
     * @param bool fade - use fade effect on brightness change
     * @param bool natural - apply dim8 function for natural brightness controll
     */
    void setBrightness(const uint8_t _tgtbrt, const bool fade=FADE, const bool natural=true);

    /*
     * Get current brightness
     * FastLED brighten8 function applied internaly for natural brightness compensation
     * @param bool natural - return compensated or absolute brightness
     */
    uint8_t getBrightness(const bool natural=true);

    /*
     * Non-blocking light fader, uses system ticker to globaly fade FastLED brighness
     * within specified duration
     * @param uint8_t _targetbrightness - end value for the brighness to fade to, FastLED dim8
     *                                   function applied internaly for natiral dimming
     * @param uint32_t _duration - fade effect duraion, ms
     */
    void fadelight(const uint8_t _targetbrightness=0, const uint32_t _duration=FADE_TIME);

private:
    uint8_t _brt, _steps;
    int8_t _brtincrement;
    Ticker _fadeTicker;
    void brightness(const uint8_t _brt, bool natural=true);
    void fader(const uint8_t _tgtbrt);
};

