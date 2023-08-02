#include <iostream>
#include "User.h"
#include "Userpool.h"
#include <vector>
#include <map>
#include "chatpool.h"
#include <stdlib.h>
#include <unistd.h>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
using namespace std;
#include <mysql.h>
#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/statement.h>
#include <cppconn/resultset.h>
#include <fstream>
#include <thread>
#include <mutex>


int main()
{
    
    MYSQL mysql;
	MYSQL_RES* res;
	MYSQL_ROW row;
 	int i = 0;
 
	// Получаем дескриптор соединения
	mysql_init(&mysql);
	if (&mysql == nullptr) {
		// Если дескриптор не получен — выводим сообщение об ошибке
		cout << "Error: can't create MySQL-descriptor" << endl;
	}
 
	// Подключаемся к серверу
	if (!mysql_real_connect(&mysql, "localhost", "root", "root", "testdb", 0, NULL, 0)) {
		// Если нет возможности установить соединение с БД выводим сообщение об ошибке
		cout << "Error: can't connect to database " << mysql_error(&mysql) << endl;
	}
    mysql_query(&mysql,"CREATE database IF NOT EXISTS testdb");
    mysql_query(&mysql,"USE testdb");
    mysql_query(&mysql,"CREATE USER IF NOT EXISTS skillfactory@localhost IDENTIFIED BY 'SkillFactory7!'");
    mysql_query(&mysql,"GRANT CREATE, SELECT, INSERT, UPDATE  ON testdb.* TO skillfactory@localhost");
    mysql_query(&mysql, "CREATE TABLE IF NOT EXISTS Users(id INT AUTO_INCREMENT PRIMARY KEY, username VARCHAR(255), password VARCHAR(255))");
    mysql_query(&mysql, "CREATE TABLE IF NOT EXISTS Messages(mes_id INT AUTO_INCREMENT PRIMARY KEY, sender VARCHAR(255), chat_id VARCHAR(10), message VARCHAR(255), mes_datetime datetime)");
 	mysql_close(&mysql);    
    
    char action('a');
    Userpool Userpool_vect;
    chatpool chatsinmap;

    while (action != 'q')
    {
        cout << "Press 1 to login with your nickname and password " << endl;
        cout << "Press 2 to create new user and send message " << endl;
        cout << "Press 3 to see all users." << endl;
        cout << "Press q to quit" << endl;
        cin >> action;

        if (action == 'q')
            break;
        string nickname;
        string password;
        string name;
        string receiver;
        string message;
        int ind(0);
        int ind_showmap(0);

        switch (action)
        {
        case '1':

            cout << "Enter your nickname" << endl;
            cin >> nickname;
            cout << "Enter your password" << endl;
            cin >> password;

            ind = Userpool_vect.checkUser(nickname, password);
            if (ind == true) {
                cout << "Enter receiver" << endl;
                cin >> receiver;
                if (receiver == nickname){
                    cout << "You cannot enter yourself as a receiver. Please try again." << endl;
                    break;
                }
                ind_showmap = chatsinmap.showmap(receiver, nickname);
                if (ind_showmap == true){
                cin.ignore();
                       
                // Запускаем функцию обработки сообщений от клиентов и ответа на эти сообщения в отдельном потоке
               
                std::thread chatThread(&chatpool::processRequest, &chatsinmap, receiver, nickname);
                chatThread.join();
                std::thread chatThread2(&chatpool::processRequest, &chatsinmap, receiver, nickname);
                chatThread2.join();
                //chatsinmap.processRequest(receiver, nickname);
                }
                else break;
                
                break;
            }
            else break;

        case '2': {

            cout << "Enter your nickname" << endl;
            cin >> nickname;
            if (nickname == "all") {
                cout << "This nickname is not allowed. Please try again." << endl;
                break;
            }
            cout << "Enter your password" << endl;
            cin >> password;

            Userpool_vect.addUser(User(nickname, password));
       
            break; }

        case '3':{

      
            Userpool_vect.showUsers();
            break;

        default: break;
        }
        
        }
    }
} 
