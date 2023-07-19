# UPS APC <-> ESP32 <-> Home Assistant
disclaimer:
Сразу скажу, что я не проф. программист, поэтому код может где то быть не элегантен.
Делал для себя.

За основу протокола: http://www.apcupsd.org/manual/#apc-smart-protocol
Cхема кабеля: http://www.apcupsd.org/manual/#id40

По итогу все работает, интервал обновления ~10 сек
Было проведено порядка 10 различных схем подключения и только на 10-11 получилось.

Что тянется:
Входное напряжение
Выходное напряжение
Напряжение батареи
Уровень заряд батареи
Оставшееся время на батарее
Нагрузка
Температура
Частота сети
Последнее сообщение с UPS
Статус bit 

Что считается в esp:
Статус: ONLINE/OFFLINE/ONBATT
Дата последнего обновления данных (время берется по NTP, нужно ESP в интернет)
Кол-во переходов с ONLINE на ONBATT (Сохраняется в EEPROM)


Использовал сделующие компоненты:

ESP32 30pin
Logic Level shifter
TTL <-> RS232
DB9 male connector

Все кроме esp покупал в pcus.ru (Не реклама)
Вот эта у меня не заработала
https://pcus.ru/modul-preobrazovaniya-ttl-uart-v-rs232?search=uart-rs232

А вот эта норм:
https://pcus.ru/max3232-preobrazovatel-rs232-db9-uart-ttl

Logic Level shifter:
https://aliexpress.ru/item/32247463702.html?spm=a2g2w.orderdetail.0.0.78f04aa6T0iH3a&sku_id=12000018838496503


Схема подключения:


Код для ESPhome в файле /esp32/ups-apc-protocol.yaml
Папку esp32/apc положить на сервер: /config/esphome/apc/





