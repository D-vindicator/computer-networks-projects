#ifndef UTIL_H
#define UTIL_H

#include <mutex>
#include <thread>
#include <vector>
#include <queue>
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
const int HEADER_SIZE = 20;
streambuf * sbuf;
ostream logout(sbuf);
mutex logout_l;
string thisip = "unknown";
string thatip = "unknown";
string thisport = "rand_port";
string thatport = "rand_port";
string thisackport = "rand_port";
string thatackport = "rand_port";

struct tcp_header{
	unsigned int s_port:16, d_port: 16, seq_num: 32,
	ack_num:32, :11, ack:1, :3, fin:1, w_size:16, ck_sum:16,emp:16;
    tcp_header(){memset(this, 1, sizeof(*this)); ack = 0; fin = 0;}
    };


string timenow()
{
    time_t now;
    time(&now);
    return ctime(&now);
}

class tcp_socket
{
private:
    int s;
public:
    tcp_socket(): s(socket(AF_INET, SOCK_STREAM, 0)){}
    
    tcp_socket(int sock) : s(sock) {}
    
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
            {
                cout<<"tcp port set to: "<<port<<endl;
                listen(s, 3);
                cout<<"start listenning..."<<endl;
            }
    }
    
    void conn(string ip_addr, int port)
    {
        sockaddr_in s_cp;
        s_cp.sin_family = AF_INET;
        s_cp.sin_addr.s_addr = inet_addr(ip_addr.c_str());
        s_cp.sin_port = htons(port);
        if (connect(s, (sockaddr*)&s_cp, sizeof(s_cp)) != 0) {
            cout<<"tcp conn error"<<endl;
        }
    }

    tcp_socket acpt()
    {
        sockaddr_in sai;
        socklen_t l = sizeof(sai);
        return tcp_socket(accept(s, (sockaddr*)&sai, &l));
    }

    
    void send_ack(int seq_num)
    {
        tcp_header ack_header;
        ack_header.ack_num = seq_num;
        ack_header.ack = 1;
        logout_l.lock();
        logout
        <<timenow()
        <<"SEND ACK: "
        <<thisip
        <<"/"
        <<thisackport
        <<" "
        <<thatip
        <<"/"
        <<thatackport
        <<" "
        <<to_string(ack_header.ack_num)
        <<" "
        <<"ACK"
        <<endl;
        logout_l.unlock();
        
        //logout<<"sent ACK:"<<ack_header.ack_num<<endl;
        if (!send(s, &ack_header, HEADER_SIZE, 0 ))
            cout<<"send error"<<endl;
    }
    
    int receive_ack()
    {
        tcp_header ack_header;
        if(recv(s, &ack_header, sizeof(ack_header), 0) == -1)
            //cout<<"receive error"<<endl;
        //cout<<"received ACK: "<<ack_header.ack_num<<endl;
        
        logout_l.lock();
        logout
        <<timenow()
        <<"RECEIVE ACK: "
        <<thatip
        <<"/"
        <<thatackport
        <<" "
        <<thisip
        <<"/"
        <<thisackport
        <<" "
        <<to_string(ack_header.ack_num)
        <<" "
        <<"ACK"
        <<endl;
        logout_l.unlock();
        
        return ack_header.ack_num;
    }
    
    void shut()
    {
        close(s);
    }
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
    
    void send_packet(tcp_header s_header, string content)
    {
        char temp_cstr[HEADER_SIZE + SEG_SIZE];
        char content_cstr[SEG_SIZE+1];
        //cout<<sizeof(content.c_str())<<"strcpy next"<<endl;
        strcpy(content_cstr, content.c_str());
        memcpy(temp_cstr, &s_header, HEADER_SIZE);
        memcpy((temp_cstr + HEADER_SIZE), content_cstr, SEG_SIZE);
        string temp = temp_cstr;
        cout<<temp.size()<<endl;
        unsigned long sum = 0;
        int i = 0;
        for (i = 0; i < temp.size(); i+=2)
        {
            int temp_sum = 0;
            string two_bytes = temp.substr(i,min(2,(int)(temp.size()- i)));
            if (two_bytes.length() == 1) {
                temp_sum += (int)two_bytes[0];
            }
            else
            {
                temp_sum += (int)two_bytes[0] * 256;
                temp_sum += (int)two_bytes[1];
            }
            
            sum = sum + temp_sum;
            //cout<<two_bytes<<":"<<temp_sum<<endl;
        }
        unsigned short remainder = sum % 65536;
        unsigned short checksum = (sum >> 16) + remainder;
        s_header.ck_sum = 65535 - checksum;
        memcpy(temp_cstr, &s_header, HEADER_SIZE);
        
        
        
        if( sendto(s, temp_cstr, HEADER_SIZE  + SEG_SIZE, 0,(sockaddr*) &cp_addr, sizeof(cp_addr)) == -1)
        	cout<<"send error"<<endl;
        else
        {
            logout_l.lock();
            logout
            <<timenow()
            <<"SEND DATA: "
            <<thisip
            <<"/"
            <<thisport
            <<" "
            <<thatip
            <<"/"
            <<thatport
            <<" "
            <<to_string(s_header.seq_num);
            if (s_header.fin == 1) {
                logout<<" "<<"FIN"<<endl;
            }
            else
                logout<<endl;
            logout_l.unlock();
        }
    }
    
    string receive_packet(tcp_header *r_header)
    {
        char buff[SEG_SIZE + HEADER_SIZE];
        char content[SEG_SIZE+2];
        unsigned int t;
        t = sizeof(cp_addr);
        //cout<<"receiving..."<<endl;
        recvfrom(s, buff , SEG_SIZE+HEADER_SIZE, 0,(sockaddr*)&cp_addr, &t);
        //cout<<r<<" bytes data received."<<endl;
        //buff[r] = '\0';
        strcpy(content, buff+HEADER_SIZE);
        content[SEG_SIZE] = '\0';
        string result = content;
        //cout<<content<<endl;
        memcpy(r_header, buff, HEADER_SIZE);
        //cout<<"content:"<<result<<endl;
        //cout<<result.length()<<endl;
        unsigned short checksum = r_header->ck_sum;
        r_header->ck_sum = 65535;
        memcpy(buff, r_header, HEADER_SIZE);
        
        
        
        string temp = buff;
        temp = temp.substr(0,SEG_SIZE+HEADER_SIZE);
        //cout<<temp.size()<<endl;
        int i = 0;
        unsigned long sum = 0;
        for (i = 0; i < temp.size(); i+=2)
        {
            int temp_sum = 0;
            string two_bytes = temp.substr(i,min(2,(int)(temp.size()- i)));
            if (two_bytes.length() == 1) {
                temp_sum += (int)two_bytes[0];
            }
            else
            {
                temp_sum += (int)two_bytes[0] * 256;
                temp_sum += (int)two_bytes[1];
            }
            sum = sum + temp_sum;
            //cout<<two_bytes<<":"<<temp_sum<<endl;
        }
        sum = sum + 257*2;
        //cout<<sum<<" "<<checksum<<" "<<i<<endl;
        
        sum = sum + checksum;
        
        if (r_header->fin == 1) {
            
            logout_l.lock();
            logout
            <<timenow()
            <<"RECEIVE DATA: "
            <<thatip
            <<"/"
            <<thatport
            <<" "
            <<thisip
            <<"/"
            <<thisport
            <<" "
            <<to_string(r_header->seq_num)
            <<" "
            <<"FIN"
            <<endl;
            logout_l.unlock();
            
            return result;
        }
        
        if (sum % 65536 + (sum >> 16) != 65535) {
            
            logout_l.lock();
            logout
            <<timenow()
            <<"RECEIVE DATA:"
            <<thatip
            <<"/"
            <<thatport
            <<" "
            <<thisip
            <<"/"
            <<thisport
            <<" "
            <<to_string(r_header->seq_num)
            <<" "
            <<"CORRUPTED"
            <<endl;
            logout_l.unlock();
            
            return receive_packet(r_header);
        }
        
        logout_l.lock();
        logout
        <<timenow()
        <<"RECEIVE DATA: "
        <<thatip
        <<"/"
        <<thatport
        <<" "
        <<thisip
        <<"/"
        <<thisport
        <<" "
        <<to_string(r_header->seq_num)
        <<endl;
        logout_l.unlock();
        
        return result;
        
    }

	void shut() {close(s);}


};







#endif