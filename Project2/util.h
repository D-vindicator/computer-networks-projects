#ifndef UTIL_H
#define UTIL_H
/*
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
#include <string.h>
#include <time.h>
#include <stdio.h>
 */
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
//const int BUFFF_SIZE = 32;
const int SEG_SIZE = 128;

struct tcp_header{
	unsigned int s_port:16, d_port: 16, seq_num: 32, 
	ack_num:32, :11, ack:1, :3, fin:1, w_size:16, ck_sum:16,:16;
};

class udp_socket
{
private:
	int s;
	sockaddr_in cp_addr;

public:
	udp_socket(): s(socket(AF_INET, SOCK_DGRAM, 0 )) {}

	udp_socket(int sock) : s(sock) {}

	int sock() {return s;}

	void set_port(int port)
	{
		sockaddr_in s_in;
		s_in.sin_family = AF_INET;
		s_in.sin_addr.s_addr = INADDR_ANY;
		s_in.sin_port = htons(port);
		if (::bind(s, (sockaddr*)&s_in, sizeof(s_in)) == -1)
			cout<<"bind error"<<endl;
		else
			cout<<"port set to: "<<port<<endl;
	}

	void set_cp(string ip_str, int port)
	{
		addrinfo *tempinfo;
		cp_addr.sin_family = AF_INET;
		cp_addr.sin_addr.s_addr = INADDR_ANY;
        if (getaddrinfo(ip_str.c_str(), to_string(port).c_str(), NULL, &tempinfo) == -1)
        	cout<<"getaddrinfo error"<<endl;
		memcpy(&cp_addr, tempinfo->ai_addr, tempinfo -> ai_addrlen);
		cout<<"counterpart info set, "<<ip_str<<": "<<port<<endl;
	}
    
    void send_packet(string content)
    {
        if( sendto(s, content.c_str(), content.length(), 0,(sockaddr*) &cp_addr, sizeof(cp_addr)) == -1)
        	cout<<"send error"<<endl;
        else
        	cout<<content.length()<<" characters sent"<<endl;
    }
    
    string receive_packet()
    {
        char buff[SEG_SIZE];
        unsigned int t;
        t = sizeof(cp_addr);
        //cout<<"receiving..."<<endl;
        int r = recvfrom(s, buff , SEG_SIZE, 0,(sockaddr*)&cp_addr, &t);
        buff[r] = '\0';
        string result = buff;
        result = result.substr(0,r);
        return result;
    }

	void shut() {close(s);}


};







#endif