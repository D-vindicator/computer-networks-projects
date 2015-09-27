#include "./util.h"


int main(int argc, char *argv[])
{
	int socket_client;
	struct sockaddr server_addr;
	struct addrinfo *serverinfo;
	char buffer[BUFFER_SIZE];


	if (argc < 3)
	{
		cout<<"inadequate input"<<endl;
		exit(1);
	}
	if ((socket_client = socket(AF_INET,SOCK_STREAM,0)) < 0)
	{
		cout<<"creating socket error"<<endl;
		exit(1);
	}
	if (getaddrinfo(argv[1], argv[2], NULL, &serverinfo) < 0)
	{
		cout<<"getaddrinfo error"<<endl;
		exit(1);
	}
	memcpy(&server_addr, serverinfo->ai_addr, serverinfo->ai_addrlen);
	if (connect(socket_client, &server_addr, sizeof(server_addr)) < 0)
	{
		cout<<"connect error"<<endl;
		exit(1);
	}

	cout<<"Enter message:"<<endl;
	bzero(buffer,BUFFER_SIZE);
	cin>>buffer;// Dangerous operation
	if (write(socket_client,buffer,strlen(buffer)) < 0)
	{
		cout<<"writing socket error!"<<endl;
		exit(1);
	}
	

	close(socket_client);
	freeaddrinfo(serverinfo);
	return 0;
}