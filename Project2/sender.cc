#include "./util.h"

using namespace std;

int window_size = 2;
mutex ack_count_l;
int ack_seq;
tcp_socket s_ack;
int un_acked_count = 0;
int ack_op = 1;
int TIME_OUT = 500;

struct packpair{
    string content;
    tcp_header header;
    int expected_ack_seq;
    packpair(string c, tcp_header h, int a) : content(c), header(h), expected_ack_seq(a){}
};
queue<packpair> unacked_sent;
mutex queue_l;

void ack_handler()
{
    while (ack_op) {
        ack_seq = s_ack.receive_ack();
//        cout<<"ack_seq:"<<ack_seq<<endl;
//        cout<<"expected:"<<unacked_sent.front().expected_ack_seq<<endl;
//        cout<<(ack_seq == unacked_sent.front().expected_ack_seq)<<endl;
        if (ack_seq == unacked_sent.front().expected_ack_seq) {
            queue_l.lock();
            unacked_sent.pop();
            queue_l.unlock();
            
            ack_count_l.lock();
            un_acked_count --;
            ack_count_l.unlock();
        }
    }
}

int main(int argc, char *argv[])
{
    if (argc != 7) {
        //cout<<argv[7]<<endl;
        cout<<"wrong input!"<<endl;
        exit(1);
    }
    
    
    string filename = argv[1];
	string remote_ip = argv[2];
	int remote_port = atoi(argv[3]);
	int ack_port = atoi(argv[4]);
	string log_file = argv[5];
    window_size = atoi(argv[6]);
    

    
    thatip = remote_ip;
    thatport = to_string(remote_port);
    thisackport = to_string(ack_port);
    
    
    
    //std::streambuf * sbuf;
    std::ofstream logstream;
    
    if(log_file != "stdout") {
        logstream.open(log_file);
        sbuf = logstream.rdbuf();
    } else {
        sbuf = std::cout.rdbuf();
    }
    
    //std::ostream out(sbuf);
    logout.rdbuf(sbuf);
    
    //logout<<timenow()<<endl;
    

    
    
    
    
    
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
    
    
    thread ack_t(ack_handler);
    ack_t.detach();
    
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
        
        queue_l.lock();
        unacked_sent.push(packpair(temp_s, s_header, s_header.seq_num + temp_s.length()));
        queue_l.unlock();
        
        un_acked_count++;
        ack_count_l.unlock();
        //cout<<"packet sent -- seq num:"<<s_header.seq_num<<endl;
        s_header.seq_num += temp_s.length();
        
        
        
        //cout<<"segment sent"<<endl;
        int l = ifs.gcount();
        if (l != SEG_SIZE)
            break;
        int time_count = 0;
        while (un_acked_count == window_size ) {
            if (time_count++ < TIME_OUT) {
                usleep(1);
            }
            else
            {
                queue<packpair> cp = unacked_sent;
                while (!cp.empty()) {
                    s.send_packet(cp.front().header, cp.front().content);
                    cp.pop();
                }
            }
        }
        
    }
    
    
    
    ifs.close();
    tcp_header fin_header;
    fin_header.fin = 1;
    s.send_packet(fin_header, "");
    

    
    if (log_file != "stdout") {
        logstream.close();
    }
	//s.send_packet("test");
    ack_op = 0;
    //ack_t.join();
	s.shut();
    s_tcp.shut();
    s_ack.shut();


	return 0;
}

