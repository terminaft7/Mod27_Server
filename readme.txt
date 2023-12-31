Комментарий к итоговому заданию по модулю 27.
Для компиляции программы сервера нужно использовать команду:
$ g++ -std=c++17 -o example.exe Chatpool.cpp Source_Server.cpp User.cpp Userpool.cpp Logger.cpp $(mysql_config --cflags --libs) -lmysqlcppconn

Метод считывания из лога в отдельном потоке запускается в main-файле Source_Server.cpp. Результат можно сравнить со считыванием из базы данных.
Метод записи сообщений в лог в отдельном потоке запускается в методе processRequest(Chatpool.cpp).
Метод считывания из лога у меня работает только с ifstream.
Программа клиента без изменений.
-------------------------------------------------------------------------------------------------------------------------------------------------
О работе проекта глобально.
В проекте две программы - сервер и клиент.

Сервер обращается к базе данных локально, в то время как клиент подключается к базе данных удалённо с помощью библиотек сppconn, mysql_driver и mysql_connection. 

Сервер создаёт базу данных, две таблицы и пользователя, через которого программа клиент подключается к серверу.
В первую таблицу Users записываются айди пользователя, никнейм и хэш пароля.
Во вторую таблицу сервером записываются сообщения, их айди номер, отправитель сообщения, дата сообщения, а также уникальный айди номер чата, который создаётся из айди отправителя, рёшетки и айди получателя. При выведении истории чата проверяются два возможных айди-номера чата - a#b и b#a. Метод находит все сообщения с таким айди-номером и выводит их, сортируя по дате.

Чтобы собрать проект, в терминале нужно задать строку

Для сервера
g++ -o example.exe Chatpool.cpp Source_Server.cpp User.cpp Userpool.cpp $(mysql_config --cflags --libs) -lmysqlcppconn

Для клиента
g++ -o example.exe Chatpool.cpp Source_Client.cpp User.cpp Userpool.cpp $(mysql_config --cflags --libs) -lmysqlcppconn


Далее подробно о программах и описании их работы.

В проекте использовано два контейнера - vector и map, и реализовано три класса: класс User, класс Chatpool и класс Userpool. Класс User нужен для хранения данных для логина (никнейм и пароль). 
Класс Userpool - это вектор, в который добавляются объекты класса User. Userpool может, соответственно, совершать некоторые простые операции с объектами: 
проверять никнейм и пароль для логина, добавлять нового пользователя и выводить список всех пользователей. 
Класс Chatpool - это класс, у объектов которого есть вектор из словарей map и ряд методов для действий с этим вектором. Такой объект с названием chatsinmap 
создаётся при запуске программы.

ПРИМЕЧАНИЕ. Эта структура с классами и контейнерами не оптимальна и унаследована от предыдущего проекта чата. Создаются пустые объекты только для того, чтобы запускать методы.

Когда пользователь в меню выбирает опцию 2 для создания нового пользователя, создаётся новый объект с именем пользователя. 
В полях этого объекта хранится никнейм и пароль пользователя. 
Также никнейм уникальный идентификационный номер, никнейм и хэш пароля записываются в mysql-таблицу Users.

ПРИМЕЧАНИЕ. Когда добавляется новый пользователь со стороны клиента, ловится SQL Exception без подробностей по ошибке, при этом запрос выполняется и новый пользователь добавляется в таблицу. Что в данном случае идёт не так разобраться не удалось, поэтому исключение просто игнорируется. 

Далее идёт возврат в основное меню. Нужно выбрать опцию 1, чтобы залогиниться и начать чат.
Когда пользователь в меню выбирает опцию 1 для отправки личных сообщений, метод Userpool'a 
проверяет наличие такого никнейма и пароля в таблице Users. Если объект с таким никнеймом и паролем есть, логин подтверждается и пользователь 
может написать адресата. Метод showmap объекта chatsinmap выводит историю чата. Затем можно ввести сообщение. 

Далее будет запущен метод processRequest(в программе сервера) или sendRequest(в программе клиента), который работает по следующему принципу:

- сначала должен зайти пользователь на сервере и написать адресата
- затем заходит пользователь с программы-клиента и пишет адресата
- если у находящихся онлайн сервера и клиента адресаты совпадают, то включается режим и чата: пользователи могут переписываться друг с другом
и сообщения сохраняются в mysql с помошью метода sendmessage, который выполняется только сервером.
- клиент отправляет end для завершения общения
- если после завершения общения пользователь захочет написать адресату, которому он уже писал, то будет выведена история чата
- если у клиента и сервера не совпадают адресаты, то соединения будет разорвано и будет предложено пообщаться попозже

После каждого приёма или отправки сообщения запускается метод sendmessage. Сначала будет создано полное сообщение, которое будет состоять из
имени отправителя, двоеточия и самого сообщения. 

Затем помощью метода checkmap будет проведена проверка, есть ли уже переписка с конкретным адресатом. 

Нажав 3, можно посмотреть никнеймы всех существующих пользователей.

Класс chatpool содержит публичный вектор chatpool_vect из словарей map<int, string>.
Имеет следующие методы: void showmap(string receiver, string nickname)- для показа переписки с конкретным пользователем; 
void sendmessage(string receiver, string nickname, string message) - метод для отправки сообщения. Этот метод запишет сообщение в базу данных. Выполняется только на сервере.
void processRequest(string receiver, string nickname) - для запуска общения между клиентом и сервером

Класс User содержит два приватных поля - nickname и password, а к ним - сеттеры и геттеры.

Класс Userpool содержит публично доступный вектор с объектами класса User и следующие методы: 
void addUser(User username) - добавить нового пользователя; 
int checkUser(string nickname, string password) - логин по никнейму и паролю; 
void showUsers()- вывод никнеймов всех существующих пользователей.
