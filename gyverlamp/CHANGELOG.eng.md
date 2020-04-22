   ### Version: featured
   **New:**
   -  Implemented async fader using scheduler, no more loop locking
        -  Natural brighness control now uses dim function to compensate for human's eye perception. Brighness 128 looks much closer to human's 50% of maximum
        - Fader effect also look much better
   -  Disabled serial output debug messages by default to save memory and space
      - debug-verson could be build via platformio target (pio run -e debug), or uncommenting _DEBUG_ definition in config.h
   -  NTPClient replaced with core's NTP
      - Timezone setup adjusts daylight saving
      - NTP-servers could be aquired via DHCP-option
   -  Enabled http based OTA, could be accessed via http://esp-xxxx/update

   **Fixes:**
   -  now alarm depends on lamp power status, if lamp is On than no need to trigger dawn function it is already pretty bright and funky :)
   -  locks elliminated in UDP-parser
   -  touch button no longer disables demo state on power on/off, next/prev effect
   -  locks elliminated in MQTT, WiFi disconnects disabled all MQTT functions and resumes on reconnect
   -  removed useless MQTT-subscriptions
   -  disabled ESP restarts in case if mqtt server goes down, it might be not an esp issue at all
   -  workaround app behavior with unnecessary config change requests that triggered mqtt/eeprom updates
   -  alarm brightness changes gradualy every second
   -  alarm is blocked if no time available via NTP
   -  fixed HomeAssistant status sync after lamp reboot

   **Other:**
   -  Adopt building with platformio with up to date core and libs
   -  all side libs has been removed out of project tree
   -  obsolete 3rd party libs, using core functions instead
   -  code refactoring, locking ellimination, using system ticker scheduler instead of millis() polling

   **Known issues:**
   -  app behaves pretty wierd with status updates
   -  sensor-button could miss some touches 
   -  HA discover config function is a real stack mem hog, its better not to use it at all that way for now

  Версия 1.5.5
   -  Синхронизированы изменения с версией 1.5.5 оригинальной прошивки
   -  Добавлено: режим "недоступно" в HomeAssistant после  обесточивания лампы
   -  Добавлено: Управление мощностью передатчика
   -  Добавлено: Запуск точки доступа с открытием портала первоначальной настройки. Для его активации нужно в течении одной минуты пять раз подать питание и обесточить лампу (включить/выключить из розетки)
   -  Добавлено: Демо режим: в демо режиме эффекты запускаются случайно по таймеру. Задать интервал обновления можно в переменной Timer *demoTimer = new Timer(60000);                                                                                                                                                           ^  - в миллисекундах
   -  Добавлено: В демо режиме пропускается эффект "смена цвета" если переменная epilepsy инициализирована в false
   -  Добавлено: Сохранение статуса лампы (включено/выключено)
   -  Добавлено: Доработан механизм плавного включения лампы
   -  Добавлено: Новыые эффекты - Аквариум, Звездопад, Пейнтбол!
   -  Добавлено: Переработан эффект смена цвета.
   -  Добавлено: Новый эффект - Спираль!

   -  Исправлено: ошибка синхронизации с Home Assistant при управлении лампой через приложение для смартфона
   -  Исправлено: "разгорание" лампы с нуля при изменении яркости из Home Assistant

   Версия 1.4 MQTT Edition:
   - Удалена настройка статического IP - статический IP лучше настраивать на стороне роутера
   - Добавлена поддержка MQTT сервера
   - Добавлено ОТА обновление через сетевой порт
   - Добавлена интеграция с Home Assistant через MQTT Discover - лампа просто появится в Home Assistant
   - Добавлена возможность выбирать цвет из RGB палитры HomeAssistant
   - Добавлено автообнаружение подключения кнопки
   - Добавлен запуск портала конфигурации при неудачном подключении к WiFi сети
   - Добавлено адаптивное подключение к MQTT серверу в случае потери соединениия
   - Добавлено принудительное включение эффекта матрицы во время OTA обновлении
   - Добавлен вывод IP адреса при пятикратном нажатии на кнопку
   - Добавлен вывод уровеня WiFi сигнала, времени непрерывной работы и причина последней перезагрузки модуля

   Версия 1.4:
   - Исправлен баг при смене режимов
   - Исправлены тормоза в режиме точки доступа
