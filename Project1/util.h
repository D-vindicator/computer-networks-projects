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
#include <map>
#include <string.h>

using namespace std;

typedef unordered_map <string,string> string_map;

static int BUFFER_SIZE = 1024;
static int CONSECUTIVE_FAILURES = 3;
static int BLOCK_TIME = 60;
enum command {REQUEST_CONNECT, REQUEST_USERINFO, USERINFO,
AUTHENTICATED, LOGIN_DENIED, LOGIN_BLOCKED

,LOGOUT, WHOELSE, BROAD_MESSAGE, BROAD_USER, WHOLAST, MESSAGE_TO

,ONLINE, OFFLINE};

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
		cout<<buffer<<endl;
		exit(1);
	}
	return stoi(firstword);
}


string get_content(char* buffer)
{
	string content_str;
	string buffer_str = buffer;
	content_str = buffer_str.substr(buffer_str.find_first_of(' '),buffer_str.length());
	return content_str;
}

string get_content(string buffer_str)
{
	return buffer_str.substr(buffer_str.find_first_of(' '),buffer_str.length());
}


class Client_user
{
public:
	string username;
	string password;
	int socket_num = -1;
	int connection_status = OFFLINE;
	//last_active_time
};

typedef unordered_map <string,Client_user> user_map;


