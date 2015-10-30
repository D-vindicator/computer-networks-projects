#include "./util.h"

using namespace std;


int main()
{
	string filename = "rdata.txt";
	int listen_port = 4119;
	string sender_ip = "127.0.0.1";
	int sender_port = 4118;
	string log_file = "r_log";
    

	udp_socket s;
	s.set_port(4119);
	s.set_cp(sender_ip,sender_port);
    ofstream ofs;
    ofs.open(filename);
    while (1) {
        string r_str = s.receive_packet();
        ofs.write(r_str.c_str(), r_str.length());
        cout<<r_str.length()<<endl;
        if (r_str.length() != SEG_SIZE)
            break;
    }
    ofs.close();
    
    
	s.shut();
	return 0;
}