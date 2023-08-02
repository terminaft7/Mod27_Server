#pragma once
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include "User.h"
using namespace std;
#include <fstream>
#include <mutex>

class chatpool {
public:
	vector<map<int, string> > chatpool_vect;
	bool showmap(string receiver, string nickname);
	void sendmessage(string receiver, string nickname, string message);
	void processRequest(string receiver, string nickname);
	void logmessage(string nickname, string receiver, string message);

protected:
	mutex mtx;
}; 
