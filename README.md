# segy
Анонимайзер для файлов формата SEG-Y.

Назначение:
- Удаляет из файла информацию, указывающую на заказчика и географические координаты.
- Затирает выбранные строки из текстового заголовка файла и смещает все координаты на фиксированное число километров по обеим осям.

Инструкция:
1. Положить все файлы для анонимизации в одну директорию. Файлы должны иметь расширение `.segy`, `.seg`, `.sgy`, `.SGY`, `.SEGY` или `.SEG`.

ВНИМАНИЕ! Файлы будут изменены, поэтому рекомендуется работать только с копиями файлов.
Необходимо также проверить, что все файлы в формате SEG-Y в заданной директории имеют права доступа "на запись" (не стоит галочка "только чтение" в свойствах файлов).

2. Из командной строки Windows запустить скрипт anonymizer.exe, передав в качестве параметров:
- название директории с файлами,
- расстояние в километрах, на которое требуется переместить все точки,
- азимут смещения,
- анономизировать групповые координаты или нет (1/0),
- анономизировать ансамблевые координаты или нет (1/0),
- (опционально) строки в текстовом заголовке для затирания (по умолчанию затираются все 40)

Например, чтобы деперсонализировать все файлы из директории `C:\data`  и сместить точки на 500 км, необходимо вызвать команду `anonymizer.exe C:\data 500`

Чтобы деперсонализировать все файлы из директории `C:\data`, при этом затирая только 10, 20 и 25 строчки текстового заголовка, необходимо вызвать команду `anonymizer.exe C:\data 500 10 20 25`

3. Все файлы в указанной папке с подходящим расширением будут деперсонализированы. В папку будут дописаны файлы `log.txt` и `errors.log`.

ВНИМАНИЕ! Если хотя бы один из файлов формата SEG-Y имеет права доступа "только чтение", ни один из
файлов не будет анонимизирован. Сообщение "The program didn't do anything!" вместе со списком
нередактируемых файлов будет дописано в `log.txt`.

ВНИМАНИЕ! Пока нет поддержки Extended Headers.
