#ifndef UTIL_H
#define UTIL_H

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
#include <algorithm>
#include <time.h>
#include <stdio.h>
#include <signal.h>

using namespace std;

typedef unordered_map <string,string> string_map;

static int BUFFER_SIZE = 1024;
static int CONSECUTIVE_FAILURES = 3;
static int BLOCK_TIME = 60;//second
static int TIME_OUT = 2;//minute

enum command {IGNORE , REQUEST_CONNECT, REQUEST_USERINFO, USERINFO,
AUTHENTICATED, LOGIN_DENIED, LOGIN_BLOCKED, CLIENT_DISP, CLIENT_LIST

,LOGOUT, WHOELSE, BROAD_MESSAGE, BROAD_USER, WHOLAST, MESSAGE_TO,

ONLINE, OFFLINE, BLOCKED, NORMAL};

istream& operator >> (istream& in, stringstream& ss)
{
    string s; in >> s; ss << s; return in;
}

void list_display(string content)
{
    for (int i = 0; i < content.size(); ++i)
	{
		if (content[i] == ' ')
			cout<<endl;
		else
			cout<<content[i];
	}
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
    content_str = buffer_str.substr(buffer_str.find_first_of(' ')+1,buffer_str.length());
	return content_str;
}

string get_content(string content)
{
    if (content.find(' ') != string::npos)
        content = content.substr(content.find_first_of(' ')+1,
                                 content.length());
    else
        content ="";
    return content;
}

class Client_user
{
	public:
    int socket_num;
	int connection_status;
    int block_status;
    time_t blocked_time;
    struct sockaddr block_address;
    time_t last_active_time;
	string username;
	string password;
    vector<string> offline_message;

    Client_user(){
        socket_num = -1;
        connection_status = OFFLINE;
        block_status = NORMAL;
    }
};

class user_map
{
public:
	map <string,Client_user> users;
	vector<string> online_users;

	void initial_user(string username, string pwd)
	{
		users[username].username = username;
		users[username].password = pwd;
	}

	int if_user_exists(string username)
	{
		return users.count(username);
	}

	int correct_password(string username, string pwd_to_check)
	{
        if (users[username].connection_status == ONLINE) {
            return 2;
        }
        return (users[username].connection_status == OFFLINE && if_user_exists(username) != 0 &&
			users[username].password == pwd_to_check);
        
	}
    
    void update_time(string username) //modify
    {
        time(&(users[username].last_active_time));
    }

    void block_user(string username) //modify
    {
        users[username].block_status = BLOCKED;
        time(&users[username].blocked_time);
    }
    
    int if_blocked(string username)
    {
        time_t now;
        time(&now);
        if (users[username].block_status == BLOCKED && difftime(now,users[username].blocked_time) < BLOCK_TIME) {
            return 1;
        }
        else
            return 0;
    }
    
	void get_online(string username,int Nsocket)//modify
	{
		users[username].connection_status = ONLINE;
		online_users.push_back(username);
		users[username].socket_num = Nsocket;
        update_time(username);
	}

	void get_offline(string username)// modify
	{
		users[username].connection_status = OFFLINE;
		online_users.erase(remove(online_users.begin(), 
			online_users.end(), username), online_users.end());
		users[username].socket_num = -1;
        update_time(username);
	}

    string get_online_user(string selfname)
    {
        stringstream ss("");
        for (int i = 0; i < online_users.size() ; i ++)
        {
            if (online_users[i] != selfname)
            {
                ss<<online_users[i];
                ss<<" ";
            }
        }
        return ss.str();
    }
    
    string get_offline_user(int duration)
    {
        time_t now;
        time(&now);
        stringstream ss;
        ss.str("");
        for (map<string,Client_user>::iterator i = users.begin(); i != users.end(); i ++)
        {
            if (((i->second).connection_status == OFFLINE) && difftime(now,(i->second).last_active_time) < duration*60)
            {
                ss<<(i->first);
                ss<<" ";
            }
        }
        return ss.str();
    }
    
    int private_message_handler(string sender, string cur_content, string &reply_content)
    {
        stringstream ss;
        ss.str(cur_content);
        string receiver;
        ss>>receiver;
        reply_content = get_content(cur_content);
        reply_content = sender+":"+reply_content;
        if (users[receiver].socket_num == -1) {
            users[receiver].offline_message.push_back(reply_content);
        }
        return users[receiver].socket_num;
    }
    
    void offline_message_handler(string username)
    {
        if (! users[username].offline_message.empty())
        {
            char buffer[BUFFER_SIZE];
            string temp;
            while (! users[username].offline_message.empty()) {
                temp = temp + " "+ users[username].offline_message.back();
            }
            integrate_message(buffer, CLIENT_DISP);
            write(users[username].socket_num, buffer, strlen(buffer));
            cout<<">>>>>>>"<<buffer<<endl;
        }
    }
};




#endif