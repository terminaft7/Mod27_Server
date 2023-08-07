#include "Logger.h"
#include <string>
#include <iostream>
#include <fstream>
#include <shared_mutex>
#include <thread>
using namespace std;

Logger::Logger()
{
    strm.open("doc1.txt", std::ios_base::out | std::ios_base::app);
    if (!strm.is_open())
    {
        cout << "File could not be opened!" << endl;
        // You might want to handle the error here or raise an exception.
    }
}

Logger::~Logger()
{
    strm.close();
    ifstrm.close();
}

void Logger::logmessage(std::string nickname, std::string receiver, std::string message)
{
    // std::unique_lock<std::shared_mutex> lock(shared_mutex);
    shared_mutex.lock();

    string log;
    log = nickname + " : " + receiver + " : " + message;

    strm << log << endl; // Add a newline after each log entry.
    shared_mutex.unlock();
    cout << "Success, check the file." << endl;
}

void Logger::logread(std::string nickname, std::string receiver)
{
    shared_mutex.lock_shared();
    ifstrm.open("doc1.txt", std::ios_base::out | std::ios_base::app);
    string line;
    string log1;
    log1 = nickname + " : " + receiver;
    string log2;
    log2 = receiver + " : " + nickname;
    ifstrm.clear();  // Reset the stream state
    ifstrm.seekg(0); // Move the read position to the beginning of the file
    if (!ifstrm.is_open())
    {
        cout << "File could not be opened!" << endl;
        return; // Return or handle the error appropriately.
    }

    while (std::getline(ifstrm, line))
    {

        if ((line.rfind(log1, 0) == 0) || (line.rfind(log2, 0) == 0))
        {
            cout << line << endl;
        }
    }
    shared_mutex.unlock_shared();
}
