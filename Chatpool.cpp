#include "chatpool.h"
#include <string>
#include <iostream>
#include <vector>
#include <stdlib.h>
#include <unistd.h>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <errno.h>
#include <mysql.h>
#include <string>
#include <fstream>
#include <mutex>

#define MESSAGE_BUFFER 4096 // Максимальный размер буфера для приема и передачи
#define PORT 7777 // Номер порта, который будем использовать для приема и передачи

char buffer[MESSAGE_BUFFER];
char send_buffer[MESSAGE_BUFFER];
int socket_file_descriptor, message_size;
socklen_t length;
const char* end_string = "end";
struct sockaddr_in serveraddress, client;
using namespace std;

string saveResultAsString(MYSQL_RES* result) { 
    string resultString; 
    if (result) { MYSQL_ROW row; unsigned int numFields = mysql_num_fields(result); 
    // Iterate over rows 
        while ((row = mysql_fetch_row(result))) { 
            // Iterate over columns 
            for (unsigned int i = 0; i < numFields; ++i) { 
                 if (row[i]) { 
                    // Append the column value to the result string 
                    resultString += row[i]; 
                     }

                } 
        } 
                mysql_free_result(result); 
    } 
    return resultString; 
} 
                     
  
bool chatpool::showmap(string receiver, string nickname) {
    MYSQL mysql;
	MYSQL_RES* res;
    MYSQL_RES* res1;
    MYSQL_RES* nickname_id;
    MYSQL_RES* receiver_id;
    
	MYSQL_ROW row;
 
	int i = 0;
 
	// Получаем дескриптор соединения
	mysql_init(&mysql);
	if (&mysql == nullptr) {
		// Если дескриптор не получен — выводим сообщение об ошибке
		cout << "Error: can't create MySQL-descriptor" << endl;
	}
 
	// Подключаемся к серверу
	if (!mysql_real_connect(&mysql, "localhost", "root", "root", "testdb", 0, 0, 0)) {
		// Если нет возможности установить соединение с БД выводим сообщение об ошибке
		cout << "Error: can't connect to database " << mysql_error(&mysql) << endl;
	}

    string check_receiver_id = "SELECT id FROM Users WHERE username LIKE  '" +  receiver +  "'";
	mysql_query(&mysql, check_receiver_id.c_str());
	receiver_id = mysql_store_result(&mysql);
    if (receiver_id != nullptr && mysql_num_rows(receiver_id) > 0){
        cout << "Chat history with " << receiver << ":" << endl;
    }

    else {
        cout << "Receiver does not exist. Chose another receiver." << endl;
        return 0;
    }

    string query_nickname_id = "SELECT id FROM Users WHERE username LIKE  '" +  nickname +  "'";
	mysql_query(&mysql, query_nickname_id.c_str());
	nickname_id = mysql_store_result(&mysql);
    string nickname_id_str;
    nickname_id_str = saveResultAsString(nickname_id);

     string query_receiver_id = "SELECT id FROM Users WHERE username LIKE '" + receiver + "'";
        if (mysql_query(&mysql, query_receiver_id.c_str()) != 0) {
            cout << "Error executing query1: " << mysql_error(&mysql) << endl;
        }

    receiver_id = mysql_store_result(&mysql);
    if (receiver_id == nullptr) {
        cout << "Error retrieving receiver ID: " << mysql_error(&mysql) << endl;
    }

    string receiver_id_str = saveResultAsString(receiver_id);
    string chat_id_opt1 = nickname_id_str + "#" + receiver_id_str;
    string chat_id_opt2 = receiver_id_str + "#" + nickname_id_str;

    string query_chat_id_opt1 = "SELECT sender, message FROM Messages WHERE chat_id LIKE '" + chat_id_opt1 + "' OR chat_id LIKE '" + chat_id_opt2 + "' ORDER BY mes_datetime";
    if (mysql_query(&mysql, query_chat_id_opt1.c_str()) != 0) {
        cout << "Error executing query: " << mysql_error(&mysql) << endl;
    }

    
      if (res1 = mysql_store_result(&mysql)) {
	    	while (row = mysql_fetch_row(res1)) {
		    	for (i = 0; i < mysql_num_fields(res1); i++) {
			    	cout << row[i] << "  ";
			    }
			    cout << endl;
		    }
	    }
	    else
		    cout << "Ошибка MySql номер " << mysql_error(&mysql);
    mysql_close(&mysql);
    return 1;
}



void chatpool::sendmessage(string receiver, string nickname, string message) {
    MYSQL mysql;
	MYSQL_RES* res;
    MYSQL_RES* nickname_id;
    MYSQL_RES* receiver_id;
	MYSQL_ROW row;
 
	int i = 0;
 
	// Получаем дескриптор соединения
	mysql_init(&mysql);
	if (&mysql == nullptr) {
		// Если дескриптор не получен — выводим сообщение об ошибке
		cout << "Error: can't create MySQL-descriptor" << endl;
	}
 
	// Подключаемся к серверу
	if (!mysql_real_connect(&mysql, "localhost", "root", "root", "testdb", 0, 0, 0)) {
		// Если нет возможности установить соединение с БД выводим сообщение об ошибке
		cout << "Error: can't connect to database " << mysql_error(&mysql) << endl;
	}

    string query_nickname_id = "SELECT id FROM Users WHERE username LIKE  '" +  nickname +  "'";
  	mysql_query(&mysql, query_nickname_id.c_str());
	nickname_id = mysql_store_result(&mysql);
	string nickname_id_str = saveResultAsString(nickname_id);
    string query_receiver_id = "SELECT id FROM Users WHERE username LIKE  '" +  receiver +  "'";
	mysql_query(&mysql, query_receiver_id.c_str());
	receiver_id = mysql_store_result(&mysql);
    string receiver_id_str = saveResultAsString(receiver_id);
    string chat_id = nickname_id_str + "#" + receiver_id_str;

  
    mysql_query(&mysql, "CREATE TABLE Messages(mes_id INT AUTO_INCREMENT PRIMARY KEY, sender VARCHAR(255), chat_id VARCHAR(10), message VARCHAR(255), mes_datetime datetime)");
    string query = "INSERT INTO Messages(mes_id, sender, chat_id, message, mes_datetime) values(default, '" + nickname + "', '"  + chat_id + "', '" +  message + "', now())";
   
    if (mysql_query(&mysql, query.c_str()) != 0) {
    // If an error occurs during the query execution
    cout << "Error executing query: " << mysql_error(&mysql) << endl;
}
      // Закрываем соединение с базой данных
	mysql_close(&mysql);
  
}


void chatpool::processRequest(string receiver, string nickname) {


    // Создадим UDP сокет
    socket_file_descriptor = socket(AF_INET, SOCK_DGRAM, 0);
    serveraddress.sin_addr.s_addr = htonl(INADDR_ANY);
    // Зададим порт для соединения
    serveraddress.sin_port = htons(PORT);
    // Используем IPv4
    serveraddress.sin_family = AF_INET;
    // Привяжем сокет
    bind(socket_file_descriptor, (struct sockaddr*)&serveraddress, sizeof(serveraddress));
    cout << "SERVER IS LISTENING THROUGH THE PORT: " << PORT << " WITHIN A LOCAL SYSTEM" << endl;
    length = sizeof(client);
    message_size = recvfrom(socket_file_descriptor, buffer, sizeof(buffer), 0, (struct sockaddr*)&client, &length);
    buffer[message_size] = '\0';
       
    //Converting receiver from string to char
 	int n = receiver.length();
    char receiver_char[n+1];
    strcpy(receiver_char, receiver.c_str());
        
    if (strcmp(receiver_char, buffer) == 0) {
        
    //Converting nickname from string to char    
        int k;
        for (k = 0; k < sizeof(nickname); k++) {
            buffer[k] = nickname[k];
        }
    //Sending nickname to client for comparison
        sendto(socket_file_descriptor, buffer, MESSAGE_BUFFER, 0, (struct sockaddr*)&client, sizeof(client));  
                     
    while (1) {
        // Длина сообщения от клиента
        length = sizeof(client);
        message_size = recvfrom(socket_file_descriptor, buffer, sizeof(buffer), 0, (struct sockaddr*)&client, &length);
        buffer[message_size] = '\0';
        if (strcmp(buffer, end_string) == 0) {
            cout << "Client left the chat or is not online. Server is Quitting" << endl;
            close(socket_file_descriptor);
        break;            
        }

        cout << "Message Received from Client: " << buffer << endl;
        this->sendmessage(nickname, receiver, buffer);
        
        //!!!!!!!!!!!!!!!!!IS THE MUTEX CORRECT?
        lock_guard<mutex> logLock1(mtx);
        this->logmessage(receiver, nickname, buffer);
        // ответим клиенту
        cout << "Enter reply message to the client: " << endl;
        cin >> send_buffer;
        sendto(socket_file_descriptor, send_buffer, MESSAGE_BUFFER, 0, (struct sockaddr*)&client, sizeof(client));
   
    //Saving message in local map container  
        this->sendmessage(receiver, nickname, send_buffer);
        lock_guard<mutex> logLock2(mtx);

        //!!!!!!!!!!!!!!!!!IS THE MUTEX CORRECT?
        this->logmessage(nickname, receiver, send_buffer);
        cout << "Waiting for the Reply from Client..!" << endl;
    }
    }
    else {cout << "Sorry, the user you want to talk to is not online. Please try later." << endl;
    char answer[2]="0";
    sendto(socket_file_descriptor, answer, MESSAGE_BUFFER, 0, (struct sockaddr*)&client, sizeof(client));  
     // закрываем сокет, завершаем соединение
    close(socket_file_descriptor);
  
    return;
    }
}

void chatpool::logmessage(string nickname, string receiver, string message) {
    fstream strm;
    strm.open("doc1.txt", ios_base::out | ios_base::app);
    if (!strm.is_open()) {
        cout << "File could not be opened!" << endl;
        return; // Return or handle the error appropriately.
    }

    string log;
    log = nickname + " : " + receiver + " : " + message;

    strm << log << endl; // Add a newline after each log entry.
    strm.close();

    cout << "Success, check the file." << endl;
}