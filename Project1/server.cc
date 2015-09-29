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
			//cout<<line<<endl;
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
		//authentication required here
		cout<<"USERINFO received"<<endl;
		//string userinfo_str =  get_content(buffer);
		stringstream ss(get_content(buffer));
		string pwd;
		ss>>username>>pwd;
		int login_count= 1;
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
			write(new_socket,buffer,1);
		}

	}
	return username;
}

int command_handler(int cur_socket, char* buffer, user_map *users)
{
	int cur_command = get_command(buffer);
	string cur_content = get_content(buffer);
	int reply_command = IGNORE;
	string reply_content;
	int reply_socket;
	if (cur_command == WHOELSE)
	{
		stringstream ss("");
		for (int i = 0; i < (*users).online_users.size() ; i ++)
		{
			ss<<(*users).online_users[i];
		}
		reply_content = ss.str();
		reply_command = CLIENT_DISP;
		reply_socket = cur_socket;
	}
	else
		return -1;
	
	if (reply_command != IGNORE)
	{
		integrate_message(buffer,reply_command,reply_content);
		write(reply_socket,buffer,strlen(buffer));
	}

	return 0;
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

		int cur_command;
		do{
			bzero(buffer,BUFFER_SIZE);
			read(new_socket, buffer, BUFFER_SIZE);
			cur_command = get_command(buffer);
			//command handler
			command_handler(new_socket, buffer, users);

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
	listen(socket_server,5);

	while(1)
	{
		socklen_t client_addr_len = sizeof(client_addr);
		int new_socket = accept(socket_server,
			(struct sockaddr *) &client_addr, &client_addr_len);
		if (new_socket >=0)
			threads.push_back(thread(client_handler, 
				&users, new_socket));
	}
	

	close(socket_server);
	return 0;
}