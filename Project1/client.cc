#include "./util.h"

vector<thread> threads;

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
    content = get_content(content);
	return result;
}

void output_handler(int socket_client)
{
    char buffer[BUFFER_SIZE];
    while (1) {
        bzero(buffer,BUFFER_SIZE);
        read(socket_client,buffer, BUFFER_SIZE);
        int cur_command = get_command(buffer);
        string cur_content = get_content(buffer);
        if (cur_command == CLIENT_LIST)
        {
            cout<<endl;
            list_display(cur_content);
            cout<<">>>";
        }
        else if (cur_command == CLIENT_DISP)
        {
            cout<<endl;
            cout<< cur_content<<endl<<">>>";
        }
        else if (cur_command == LOGOUT)
            return;
    }
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
    threads.push_back(thread(output_handler,socket_client));
	int cur_command = -1;
    cin.ignore();
	while(1)
	{
		string cur_content;
        cout<<">>>";
        getline(cin, cur_content);
		cur_command = command_parser(cur_content);
		if (cur_command >= 0 &&cur_command != LOGOUT)
		{
			integrate_message(buffer, cur_command, cur_content);
			write(socket_client,buffer,strlen(buffer));
//            if (cur_command == WHOELSE or cur_command == WHOLAST) {
//                bzero(buffer,BUFFER_SIZE);
//                read(socket_client,buffer, BUFFER_SIZE);
//                command_handler(buffer);
//            }
		}
		else
			{
				integrate_message(buffer, LOGOUT);
				write(socket_client,buffer,strlen(buffer));
				cout<<"client logged out"<<endl;
				break;
			}
	}
    
    threads[1].join();
	close(socket_client);
	freeaddrinfo(serverinfo);
	return 0;
}

