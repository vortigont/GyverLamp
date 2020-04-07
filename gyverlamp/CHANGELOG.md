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