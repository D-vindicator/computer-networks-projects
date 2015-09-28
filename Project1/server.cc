#include "./util.h"

void user_pass_handler(string_map &user_pass_map)
{
	ifstream user_pass("user_pass.txt");
	if(user_pass.is_open())
	{
		string line, username, pwd;
		string_map_element line_pair;
		while(getline(user_pass,line))
		{
			//cout<<line<<endl;
			stringstream ss(line);
			ss>>username>>pwd;
			user_pass_map[username] = pwd;
		}
		user_pass.close();
	}
	else
	{
		cout<<"unable to open user_pass.txt"<<endl;
		exit(1);
	}
}

void client_handler(string_map &user_pass_map,int &socket_server, 
	struct sockaddr_in &server_addr,struct sockaddr_in &client_addr)
{
	socklen_t client_addr_len = sizeof(client_addr);
	int new_socket = accept(socket_server,(struct sockaddr *) &client_addr, &client_addr_len);
	if (new_socket < 0)
	{cout<<"accept error"<<endl;exit(1);}


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
		bzero(buffer,BUFFER_SIZE);
		read(new_socket, buffer, BUFFER_SIZE);
		if (get_command(buffer) == USERINFO)
		{
			//authentication required here
			cout<<"USERINFO received"<<endl;
			//string userinfo_str =  get_content(buffer);
			stringstream ss(get_content(buffer));
			string username, pwd;
			ss>>username>>pwd;
			while(!(user_pass_map.count(username) != 0 && user_pass_map[username] == pwd))
			{
				integrate_message(buffer,LOGIN_DENIED);
				cout<<"client login denied"<<endl;
				write(new_socket, buffer, 1);
				bzero(buffer,BUFFER_SIZE);
				read(new_socket, buffer, BUFFER_SIZE);
				stringstream ss(get_content(buffer));
				ss>>username>>pwd;
			}

			integrate_message(buffer,AUTHENTICATED);
			cout<<"client authenticated"<<endl;
			write(new_socket, buffer, 1);
		}
	}
	close(new_socket);
}

int main(int argc, char *argv[])
{
	vector<thread> threads;
	string_map user_pass_map;

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
	if (::bind(socket_server, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
	{ cout<<"bind error"<<endl; exit(1);}
	cout<<"Server starts listening..."<<endl;
	listen(socket_server,5);

	client_handler(user_pass_map,socket_server, 
		server_addr,client_addr);

	close(socket_server);
	return 0;
}