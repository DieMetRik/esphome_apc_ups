# UPS APC <-> ESP32 <-> Home Assistant

**disclaimer:** Сразу скажу, что я не проф. программист, поэтому код может где то быть не элегантен.

Мой UPS: **APC by Schneider Electric Smart-UPS SUA1500I**, Имеет на борту USB и RS232

За основу протокола: http://www.apcupsd.org/manual/#apc-smart-protocol

Cхема кабеля: http://www.apcupsd.org/manual/#id40

По итогу все работает, интервал обновления ~10 сек

Было проведено порядка 10 различных схем подключения и только на 10-11 получилось.

**Что тянется:**
* Входное напряжение
* Выходное напряжение
* Напряжение батареи
* Уровень заряд батареи
* Оставшееся время на батарее
* Нагрузка
* Температура
* Частота сети
* Последнее сообщение с UPS
* Статус bit 

**Что считается и передает в esp32:**
* Статус: ONLINE/OFFLINE/ONBATT
* Дата последнего обновления данных (время берется по NTP, нужно ESP в интернет)
* Кол-во переходов с ONLINE на ONBATT (Сохраняется в EEPROM)

Использовал сделующие компоненты:
* ESP32 38pin
* Logic Level shifter
* TTL <-> RS232
* DB9 male connector

Покупал тут:
* Esp32: https://aliexpress.ru/item/32864722159.html
* Logic Level shifter: https://aliexpress.ru/item/32247463702.html
* Вот эта у меня не заработала: https://pcus.ru/modul-preobrazovaniya-ttl-uart-v-rs232
* А вот эта норм: https://pcus.ru/max3232-preobrazovatel-rs232-db9-uart-ttl

Схема подключения:
![Image alt](https://github.com/DieMetRik/esphome_apc_ups/blob/main/image/APC-UPS.png)

Кабель:

![Image alt](https://github.com/DieMetRik/esphome_apc_ups/blob/main/image/940-0024C.png)

Код для ESPhome в файле /esp32/ups-apc-protocol.yaml
Папку /esp32/apc положить на сервер: /config/esphome/apc/

Пример для Home Assistant:

![Image alt](https://github.com/DieMetRik/esphome_apc_ups/blob/main/image/ha_apc_preview.PNG)

