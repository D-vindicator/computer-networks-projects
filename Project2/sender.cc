#include "./util.h"

using namespace std;

int window_size = 2;
mutex ack_count_l;
int ack_seq;
tcp_socket s_ack;
int un_acked_count = 0;
int ack_op = 1;


void ack_handler(vector<int> *acked_vec)
{
    while (ack_op) {
        ack_seq = s_ack.receive_ack();
        ack_count_l.lock();
        un_acked_count --;
        acked_vec->push_back(ack_seq);
        ack_count_l.unlock();
    }
}

int main(int argc, char *argv[])
{
	string filename = "data.txt";
	string remote_ip = "127.0.0.1";
	int remote_port = 20000;
	int ack_port = 10001;
	string log_file = "logfile.txt";
	
    int udp_send_port = 10000;

    udp_socket s;
    s.set_port(udp_send_port);
    s.set_cp(remote_ip,remote_port);
    
    tcp_socket s_tcp;
    s_tcp.set_port(ack_port);
    cout<<"wating for receiver..."<<endl;
    s_ack = s_tcp.acpt();
    cout<<"receiver accepted"<<endl;

	ifstream ifs;
    ifs.open(filename.c_str());

    char seg_buff[SEG_SIZE];
    
    tcp_header s_header;
    s_header.s_port = 4118;
    s_header.d_port = 4119;
    
    
    vector<int> acked_vec;
    thread ack_t(ack_handler,&acked_vec);
    
    while (un_acked_count < window_size) {
        int s_index = ifs.tellg();
        ifs.seekg(0,ifs.end);
        int e_index = ifs.tellg();
        ifs.seekg(s_index);
        int cur_size = min(SEG_SIZE, e_index - s_index);
        ifs.read(seg_buff, SEG_SIZE);
        string temp_s = seg_buff;
        temp_s = temp_s.substr(0,cur_size);
        
        ack_count_l.lock();
        s.send_packet(s_header, temp_s);
        un_acked_count++;
        ack_count_l.unlock();
        
        s_header.seq_num += temp_s.length();
        cout<<"packet sent -- seq num:"<<s_header.seq_num<<endl;
        
        
        //cout<<"segment sent"<<endl;
        int l = ifs.gcount();
        if (l != SEG_SIZE)
            break;
        
        while (un_acked_count == window_size) {
            usleep(100);
        }
    }
    
    
    
    ifs.close();
    tcp_header fin_header;
    fin_header.fin = 1;
    s.send_packet(fin_header, "");
    

	//s.send_packet("test");
    ack_op = 0;
    ack_t.join();
	s.shut();
    s_tcp.shut();
    s_ack.shut();


	return 0;
}

