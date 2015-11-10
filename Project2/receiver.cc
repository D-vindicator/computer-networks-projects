#include "./util.h"

using namespace std;


int main(int argc, char *argv[])
{
    if (argc != 6) {
        cout<<"wrong input!"<<endl;
        exit(1);
    }
    
    string filename = argv[1];
	int listen_port = atoi(argv[2]);
	string sender_ip = argv[3];
	int sender_port = atoi(argv[4]);
	string log_file = argv[5];
    
    
    int udp_sender_port = 10000;
    
    thatip = sender_ip;
    thatackport = to_string(sender_port);
    thisport = to_string(listen_port);
    
    std::ofstream logstream;
    
    if(log_file != "stdout") {
        logstream.open(log_file);
        sbuf = logstream.rdbuf();
    } else {
        sbuf = std::cout.rdbuf();
    }
    
    //std::ostream out(sbuf);
    logout.rdbuf(sbuf);
    
    
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
    //cout<<"base_seq:"<<base_num<<endl;
    while (1) {
        string r_str = s.receive_packet(&r_header);
        if (r_header.seq_num == base_num) {
            s_tcp.send_ack(base_num + r_str.length());
            base_num +=r_str.length();
            ofs.write(r_str.c_str(), r_str.length());
            //cout<<"written content length: "<<r_str.length()<<endl;
            //cout<<"base number: "<<base_num<<endl;
            if (r_str.length() != SEG_SIZE)
                break;
        }
    }
    ofs.close();
    
    while (r_header.fin != 1) {
        string r_str = s.receive_packet(&r_header);
    }
    
    if (log_file != "stdout") {
        logstream.close();
    }

	s.shut();
    s_tcp.shut();
	return 0;
}