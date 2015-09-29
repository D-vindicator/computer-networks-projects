#include "./util.h"

int command_parser(string &content)
{
	string firstword, temp_str;
	stringstream ss;
	ss.str(content);
	int result = -1;

	ss>>firstword;
	if(firstword.empty())
		exit(1);

	if (firstword == "logout")
		result = LOGOUT;
	else if(firstword == "whoelse")
		result = WHOELSE;
	else if(firstword == "wholast")
		result = WHOLAST;
	else if(firstword == "message")
		result = MESSAGE_TO;
	else if(firstword == "broadcase")
	{
		ss>>temp_str;
		if (temp_str == "message")
			result = BROAD_MESSAGE;
		else if(temp_str == "user")
			result = BROAD_USER;
	}
	else
		cout<<"undefined command"<<endl;
 
	if (content.find(' ') != string::npos)
		content = content.substr(content.find_first_of(' '),content.length());
	else
		content ="";

	return result;
}



int main(int argc, char *argv[])
{
	int socket_client;
	struct sockaddr server_addr;
	struct addrinfo *serverinfo;
	char buffer[BUFFER_SIZE];
	stringstream ss;


	if (argc < 3)
	{ cout<<"inadequate input"<<endl; exit(1);}
	if ((socket_client = socket(AF_INET,SOCK_STREAM,0)) < 0)
	{ cout<<"creating socket error"<<endl; exit(1);}
	if (getaddrinfo(argv[1], argv[2], NULL, &serverinfo) < 0)
	{ cout<<"getaddrinfo error"<<endl; exit(1); }

	memcpy(&server_addr, serverinfo->ai_addr, serverinfo->ai_addrlen);
	if (connect(socket_client, &server_addr, sizeof(server_addr)) < 0)
	{ cout<<"connect error"<<endl; exit(1); }

	integrate_message(buffer,REQUEST_CONNECT);
	write(socket_client,buffer,strlen(buffer));
	cout<<"REQUEST_CONNECT sent"<<endl;

	bzero(buffer,BUFFER_SIZE);
	read(socket_client, buffer, BUFFER_SIZE);
	//some timeout function should be here later

	if (get_command(buffer) != REQUEST_USERINFO)
	{
		cout<<"Unexpected server bahavior."<<endl;
		exit(1);
	}

	do{
		if(get_command(buffer) == LOGIN_DENIED)
			cout<<"Wrong username or password"<<endl;
		cout<<"Input your username and password"<<endl;
		ss.str("");
		cout<<"Username: ";
		cin >> ss;
		ss<<" ";
		cout<<"Password: ";
		cin >> ss;
		integrate_message(buffer, USERINFO, ss.str());
		write(socket_client,buffer,strlen(buffer));
		bzero(buffer,BUFFER_SIZE);
		read(socket_client,buffer, BUFFER_SIZE);
		if (get_command(buffer) == LOGIN_BLOCKED)
		{cout<<"login is blocked for some time, please restart the client"<<endl; exit(1);}
	} while(get_command(buffer) == LOGIN_DENIED);


	cout<<"Welcome to Simiple Chat!"<<endl;
	cout<<"Input your command to start!"<<endl;

	int cur_command = -1;
	while(1)
	{
		cout<<">>>";
		ss.str("");
		cin>>ss;
		string temp_str, cur_content;
		ss>>cur_content;
		cur_command = command_parser(cur_content);
		if (cur_command >= 0 &&cur_command != LOGOUT)
		{
			integrate_message(buffer, cur_command, cur_content);
			write(socket_client,buffer,strlen(buffer));
			bzero(buffer,BUFFER_SIZE);
			read(socket_client,buffer, BUFFER_SIZE);
		}
		else
			{
				integrate_message(buffer, cur_command, cur_content);
				write(socket_client,buffer,strlen(buffer));
				cout<<"client logged out"<<endl;
				break;
			}
		
	}

	close(socket_client);
	freeaddrinfo(serverinfo);
	return 0;
}

