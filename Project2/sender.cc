#include "./util.h"

using namespace std;
int main(int argc, char *argv[])
{
	string filename = "data.txt";
	string remote_ip = "127.0.0.1";
	int remote_port = 4119;
	int ack_port = 4120;
	string log_file = "logfile.txt";
	int window_size = 10;

    udp_socket s;
    s.set_port(4118);
    s.set_cp(remote_ip,remote_port);

	ifstream ifs;
    ifs.open(filename.c_str());
//    int r  = SEG_SIZE;
//    int i = ifs.tellg();
//    ifs.seekg(ifs.end);
//    int l = ifs.tellg();
//    l = min(l-i, r);
    char seg_buff[SEG_SIZE];
    while (1) {
        int s_index = ifs.tellg();
        ifs.seekg(0,ifs.end);
        int e_index = ifs.tellg();
        ifs.seekg(s_index);
        int cur_size = min(SEG_SIZE, e_index - s_index);
        ifs.read(seg_buff, SEG_SIZE);
        string temp_s = seg_buff;
        temp_s = temp_s.substr(0,cur_size);
        s.send_packet(temp_s);
        //cout<<"segment sent"<<endl;
        int l = ifs.gcount();
        if (l != SEG_SIZE)
            break;
        
    }
    ifs.close();
    

    



	//s.send_packet("test");
	s.shut();


	return 0;
}

