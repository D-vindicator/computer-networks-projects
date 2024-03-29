#include "./util.h"

vector<thread> threads;
user_map users;
mutex user_map_lock;

void user_pass_handler(user_map &users)
{
	ifstream user_pass("user_pass.txt");
	if(user_pass.is_open())
	{
		string line, username, pwd;
		Client_user client_user;
		while(getline(user_pass,line))
		{
			stringstream ss(line);
			ss>>username>>pwd;
			users.initial_user(username,pwd);
		}
		user_pass.close();
	}
	else
	{
		cout<<"unable to open user_pass.txt"<<endl;
		exit(1);
	}
}

string login_handler(char* buffer, user_map *users, int new_socket)
{
	string username;
	bzero(buffer,BUFFER_SIZE);
	read(new_socket, buffer, BUFFER_SIZE);
	if (get_command(buffer) == USERINFO)
	{
		cout<<"USERINFO received"<<endl;
		stringstream ss(get_content(buffer));
		string pwd;
		ss>>username>>pwd;
		int login_count= 1;
        if ((*users).if_blocked(username)) {
            integrate_message(buffer, LOGIN_BLOCKED);
            write(new_socket, buffer, strlen(buffer));
            return "";
        }
		while(!(*users).correct_password(username,pwd) and
			 login_count < CONSECUTIVE_FAILURES)
		{
			login_count++;
			integrate_message(buffer,LOGIN_DENIED);
			cout<<"LOGIN_DENIED"<<endl;
			write(new_socket, buffer, 1);
			bzero(buffer,BUFFER_SIZE);
			read(new_socket, buffer, BUFFER_SIZE);
			stringstream ss(get_content(buffer));
			ss>>username>>pwd;
		}
        if ((*users).correct_password(username,pwd) == 2) {
            integrate_message(buffer, LOGIN_BLOCKED);
            write(new_socket, buffer, 1);
            return "";
        }
		if ((*users).correct_password(username,pwd))
		{
			user_map_lock.lock();
			(*users).get_online(username,new_socket);
			user_map_lock.unlock();
			integrate_message(buffer,AUTHENTICATED);
			cout<<"AUTHENTICATED"<<endl;
			write(new_socket, buffer, 1);
		}
		else
		{
			integrate_message(buffer,LOGIN_BLOCKED);
			cout<<"LOGIN_BLOCKED"<<endl;
            user_map_lock.lock();
            (*users).block_user(username);
            user_map_lock.unlock();
			write(new_socket,buffer,1);
            username = "";
		}
	}
	return username;
}

void command_handler(string selfname, int cur_socket, char* buffer, user_map *users)
{
    user_map_lock.lock();
    (*users).update_time(selfname);
    user_map_lock.unlock();
    int cur_command = get_command(buffer);
	string cur_content = get_content(buffer);
	int reply_command = IGNORE;
	string reply_content;
	int reply_socket = -1;
	if (cur_command == WHOELSE)
	{
		reply_content = (*users).get_online_user(selfname);
		reply_command = CLIENT_LIST;
		reply_socket = cur_socket;
        integrate_message(buffer,reply_command,reply_content);
        write(reply_socket,buffer,strlen(buffer));
	}
    else if (cur_command == WHOLAST)
    {
        int duration = 0;
        stringstream ss;
        ss.str(cur_content);
        ss>>duration;
        reply_socket = cur_socket;
        reply_command = CLIENT_LIST;
        reply_content = (*users).get_online_user(selfname) + (*users).get_offline_user(duration);
        integrate_message(buffer,reply_command,reply_content);
        write(reply_socket,buffer,strlen(buffer));
    }
	else if (cur_command == MESSAGE_TO)
    {
        reply_command = CLIENT_DISP;
        user_map_lock.lock();
        reply_socket = (*users).private_message_handler(selfname, cur_content, reply_content);
        user_map_lock.unlock();
        if (reply_socket >= 0) {
            integrate_message(buffer,reply_command,reply_content);
            write(reply_socket,buffer,strlen(buffer));
        }

    }
    else if (cur_command == BROAD_MESSAGE)
    {
        reply_command = CLIENT_DISP;
        reply_content = get_content(cur_content);
        reply_content = selfname+":"+reply_content;
        for (int i = 0; i < (*users).online_users.size(); i++)
        {
            reply_socket = (*users).users[(*users).online_users[i]].socket_num;
            string it_name = (*users).online_users[i];
            if (it_name != selfname && !(*users).users[it_name].in_blacklist(selfname))
            {
                integrate_message(buffer, reply_command, reply_content);
                write(reply_socket, buffer, strlen(buffer));
            }
        }
    }
    else if (cur_command == BROAD_USER)
    {
        reply_command = CLIENT_DISP;
        cur_content = get_content(cur_content);
        stringstream ss;
        ss.str(cur_content);
        string temp;
        ss>>temp;
        vector<string> receivers;
        while (temp != "message")
        {
            if (find((*users).online_users.begin(), (*users).online_users.end(), temp) != (*users).online_users.end())
            {
                receivers.push_back(temp);
            }
            ss >> temp;
        }
        getline(ss, reply_content);
        reply_content = reply_content.substr(1,reply_content.length());
        reply_content = selfname + ":" + reply_content;
        for (int i = 0; i < receivers.size(); i ++) {
            if (!(*users).users[receivers[i]].in_blacklist(selfname))
            {
                integrate_message(buffer, CLIENT_DISP,reply_content);
                write((*users).users[receivers[i]].socket_num, buffer, strlen(buffer));
            }
        }
    }
    else if (cur_command == BLACK_ADD)
    {
        user_map_lock.lock();
        (*users).users[selfname].black_enlist(cur_content);
        user_map_lock.unlock();
    }
    else if (cur_command == BLACK_REMOVE)
    {
        user_map_lock.lock();
        (*users).users[selfname].black_unenlist(cur_content);
        user_map_lock.unlock();
    }
    
}

void client_handler(user_map *users, int new_socket)
{
	char buffer[BUFFER_SIZE];
	bzero(buffer,BUFFER_SIZE);
	if (read(new_socket, buffer, BUFFER_SIZE) < 0)
	{cout<<"reading new_socket error!"<<endl;exit(1);}
	if (get_command(buffer) == REQUEST_CONNECT)
	{
		cout<<"REQUEST_CONNECT received."<<endl;
		integrate_message(buffer,REQUEST_USERINFO);
		write(new_socket, buffer, 1);
		cout<<"REQUEST_USERINFO sent."<<endl;
		string username = login_handler(buffer,users,new_socket);
        if (username == "")
        {
            close(new_socket);
            return;
        }
		int cur_command;
        usleep(100);
        user_map_lock.lock();
        (*users).offline_message_handler(username);
        user_map_lock.unlock();
		do{
			bzero(buffer,BUFFER_SIZE);
			read(new_socket, buffer, BUFFER_SIZE);
			cur_command = get_command(buffer);
            time_t now;
            time(&now);
            if (difftime(now, (*users).users[username].last_active_time) > TIME_OUT * 60)
                break;
			command_handler(username, new_socket, buffer, users);
		}while(cur_command != LOGOUT);
		user_map_lock.lock();
		(*users).get_offline(username);
		user_map_lock.unlock();
	}
	close(new_socket);
}

int main(int argc, char *argv[])
{
	user_pass_handler(users);
	int socket_server;
    signal(SIGPIPE, SIG_IGN);
	if(argc < 2)
	{ cout<<"inadequate input"<<endl;exit(1);}
	if ((socket_server = socket(AF_INET,SOCK_STREAM,0)) < 0)
	{cout<<"creating socket error"<<endl;exit(1);}
	struct sockaddr_in server_addr, client_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(atoi(argv[1]));
	if (::bind(socket_server, 
		(struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
	{ cout<<"bind error"<<endl; exit(1);}
	cout<<"Server starts listening..."<<endl;
	listen(socket_server,5);//Initialization finished, start listening
	while(1)
	{
		socklen_t client_addr_len = sizeof(client_addr);
		int new_socket = accept(socket_server,
			(struct sockaddr *) &client_addr, &client_addr_len);
		if (new_socket >=0)
			threads.push_back(thread(client_handler, 
				&users, new_socket));
	}
	return 0;
}