#pragma once
#include <string> // Include the string header
#include <iostream>
#include <fstream>
#include <shared_mutex>
#include <thread>
using namespace std;

class Logger
{
public:
    void logmessage(std::string nickname, std::string receiver, std::string message);
    void logread(std::string nickname, std::string receiver);
     
    Logger();  // Constructors
    ~Logger(); // Destructor

private:
    std::fstream strm;
    std::ifstream ifstrm;
    std::shared_mutex shared_mutex;
};


