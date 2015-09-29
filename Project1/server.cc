#include "./util.h"

vector<thread> threads;
user_map user_pass_map;
mutex user_map_lock;


void user_pass_handler(user_map &user_pass_map)
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
			user_pass_map[username].username = username;
			user_pass_map[username].password = pwd;
		}
		user_pass.close();
	}
	else
	{
		cout<<"unable to open user_pass.txt"<<endl;
		exit(1);
	}
}

string login_handler(char* buffer, user_map *user_pass_map, int new_socket)
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
		while(!((*user_pass_map).count(username) != 0 && 
			(*user_pass_map)[username].password == pwd) and
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

		if ( (*user_pass_map).count(username) != 0 && 
			(*user_pass_map)[username].password == pwd )
		{
			user_map_lock.lock();
			(*user_pass_map)[username].connection_status = ONLINE;
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

void client_handler(user_map *user_pass_map, int new_socket)
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

		string username = login_handler(buffer,user_pass_map,new_socket);

		int cur_command;
		do{
			bzero(buffer,BUFFER_SIZE);
			read(new_socket, buffer, BUFFER_SIZE);
			cur_command = get_command(buffer);
			//command handler
		}while(cur_command != LOGOUT);
		user_map_lock.lock();
		(*user_pass_map)[username].connection_status = OFFLINE;
		user_map_lock.unlock();
	}
	close(new_socket);
}

int main(int argc, char *argv[])
{
	user_pass_handler(user_pass_map);

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
		//client_handler(user_pass_map,new_socket);
		if (new_socket >=0)
			threads.push_back(thread(client_handler, 
				&user_pass_map, new_socket));
	}
	

	close(socket_server);
	return 0;
}