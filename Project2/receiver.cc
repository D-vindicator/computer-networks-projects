#include "./util.h"

using namespace std;


int main()
{
	string filename = "rdata.txt";
	int listen_port = 20000;
	string sender_ip = "127.0.0.1";
	int sender_port = 10001;
	string log_file = "r_log";
    int udp_sender_port = 10000;
    

	udp_socket s;
	s.set_port(listen_port);
	s.set_cp(sender_ip,udp_sender_port);

	tcp_socket s_tcp;
    s_tcp.conn(sender_ip, sender_port);
	
    ofstream ofs;
    ofs.open(filename);
    tcp_header r_header;
    int base_num = r_header.seq_num;
    r_header.seq_num --;
    int not_yet_conn = 1;
    //cout<<"base_seq:"<<base_num<<endl;
    while (1) {
        string r_str = s.receive_packet(&r_header);
        if (r_header.seq_num == base_num) {
            base_num +=r_str.length();
            s_tcp.send_ack(base_num);
            
            ofs.write(r_str.c_str(), r_str.length());
            cout<<"written content length: "<<r_str.length()<<endl;
            //cout<<"base number: "<<base_num<<endl;
            if (r_str.length() != SEG_SIZE)
                break;
        }
    }
    ofs.close();
    
    while (r_header.fin != 1) {
        string r_str = s.receive_packet(&r_header);
    }

	s.shut();
    s_tcp.shut();
	return 0;
}