#include "./util.h"


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



/*
	if (get_command(buffer) == REQUEST_USERINFO)
	{
		cout<<"Server requires user information."<<endl;
		ss.str("");
		cout<<"Username: ";
		cin >> ss;
		ss<<" ";
		cout<<"Password: ";
		cin >> ss;
		integrate_message(buffer, USERINFO, ss.str());
		cout<<buffer<<endl;
		write(socket_client,buffer,strlen(buffer));
	}
	else
	{
		cout<<"Unexpected server bahavior."<<endl;
		exit(1);
	}

	bzero(buffer,BUFFER_SIZE);
	read(socket_client,buffer, BUFFER_SIZE);

	if (get_command(buffer) == AUTHENTICATED)
	{
		cout<<"Welcome to Simiple Chat!"<<endl;
		cout<<"Input your command to start!"<<endl;
		cout<<">>>";
	}
	else if(get_command(buffer) == LOGIN_DENIED)
	{
		cout<<"Wrong username/password, try again"<<endl;
		ss.str("");
		cout<<"Username: ";
		cin >> ss;
		ss<<" ";
		cout<<"Password: ";
		cin >> ss;
		integrate_message(buffer, USERINFO, ss.str());
		cout<<buffer<<endl;
		write(socket_client,buffer,strlen(buffer));
	}
*/
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
	} while(get_command(buffer) != AUTHENTICATED);


	cout<<"Welcome to Simiple Chat!"<<endl;
	cout<<"Input your command to start!"<<endl;
	cout<<">>>";





	close(socket_client);
	freeaddrinfo(serverinfo);
	return 0;
}