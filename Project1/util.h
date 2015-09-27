#include <mutex>
#include <thread>
#include <vector>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <unordered_map>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <stdlib.h>

using namespace std;

static int BUFFER_SIZE = 1024;
enum command {REQUEST_CONNECT, REQUEST_USERINFO, USERINFO,
AUTHENTICATED};


istream& operator >> (istream& in, stringstream& ss){
string s; in >> s; ss << s; return in;
}


void integrate_message(char* buffer, int cmd)
{
	stringstream ss;
	bzero(buffer,BUFFER_SIZE);
	ss.str("");
	ss<<cmd<<" ";
	strcpy(buffer,ss.str().c_str());
}

void integrate_message(char* buffer, int cmd, string content)
{
	stringstream ss;
	bzero(buffer,BUFFER_SIZE);
	ss.str("");
	ss<<cmd<<" "<<content;
	strcpy(buffer,ss.str().c_str());
}

int get_command(char* buffer)
{
	string buffer_str = buffer;
	string firstword;
	if (buffer_str.find(' ') != string::npos)
	{
		firstword = buffer_str.substr(0, buffer_str.find_first_of(' '));
	}
	else
		firstword = buffer_str;
	if(firstword.empty())
	{
		cout<<"error: empty command!"<<endl;
		exit(1);
	}
	return stoi(firstword);
}
