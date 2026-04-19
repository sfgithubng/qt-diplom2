### Описание

Программа indexer предназначена для обработки файлов в определенной директории с целью построения поискового индекса на основе содержимого файлов. Поддерживаются текстовые файлы, включающие слова из кириллического и латинского алфавитов.

### Создание файла конфигурации

Для работы с программой необходимо скопировать файл config_default.ini в config.ini, сохраняя формат файла, приведенный ниже. При этом:
- Строки комментариев в файле должны начинаться со знака #;
- Файл должен находиться в той же директории, что и программа;
- Значение "file_extensions" должно быть или пустое для индексирования всех типов файлов, или содержать расширения файлов разделенные запятой или запятой и пробелом; допустимые значения для расширений - символы латинского алфавита, цифры и нижнее подчеркивание;
- Значения параметров конфигурации должны быть в одинарных кавычках согласно примеру ниже; значения в двойных кавычках или без них могут привести к использованию параметров по умолчанию.

Пример файла конфигурации:
```
[indexing]
file_extensions = 'txt, md, c'
index_directory = '/home/xubu2404-software/Documents/Netology_repos/qt-diplom2/sample_text_files'

[db]
db_host = '127.0.0.1'
db_port = '5432'
db_name = 'indexer'
db_user = 'admin'
db_pass = 'admin123'
```

### Создание Базы Данных

Пример конфигурации для GNU/Linux:
```
adminuser@dev-machine:~$ sudo -u postgres psql
[sudo] password for adminuser:
psql (16.13 (Ubuntu 16.13-0ubuntu0.24.04.1))
Type "help" for help.

postgres=# CREATE DATABASE indexer;
CREATE DATABASE
postgres=# CREATE USER indexer_admin WITH ENCRYPTED PASSWORD 'strongPASSWORDhere';
CREATE ROLE
postgres=# GRANT ALL PRIVILEGES ON DATABASE indexer TO indexer_admin;
GRANT
postgres=# ALTER DATABASE indexer OWNER TO indexer_admin;
ALTER DATABASE
postgres=# \q
adminuser@dev-machine:~$
```

### Работа с программой

Программа производит индексирование автоматически, непосредственно после запуска. В случае успешного индексирования, программа завершает работу без вывода сообщений. В случае ошибок индексирования, доступа к БД, чтения файла конфигурации, программа выводит соответствующее сообщение и прекращает работу.
