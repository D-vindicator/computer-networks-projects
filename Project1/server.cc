#include "./util.h"

int main(int argc, char *argv[])
{
	int socket_server, new_socket, port_no;
	struct sockaddr_in server_addr, client_addr;
	char buffer[BUFFER_SIZE];
	socklen_t client_addr_len = sizeof(client_addr);

	if(argc < 2)
	{
		cout<<"inadequate input"<<endl;
		exit(1);
	}
	port_no = atoi(argv[1]);

	if ((socket_server = socket(AF_INET,SOCK_STREAM,0)) < 0)
	{
		cout<<"creating socket error"<<endl;
		exit(1);
	}

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(port_no);
	if (::bind(socket_server, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
	{
		cout<<"bind error"<<endl;
		exit(1);
	}

	listen(socket_server,5);
	new_socket = accept(socket_server,(struct sockaddr *) &client_addr, &client_addr_len);
	if (new_socket < 0)
	{
		cout<<"accept error"<<endl;
		exit(1);
	}

	bzero(buffer,BUFFER_SIZE);
	if (read(new_socket, buffer, BUFFER_SIZE) < 0)
	{
		cout<<"reading socket error!"<<endl;
		exit(1);
	}
	cout<<"Received message: "<<buffer<<endl;

	if (write(new_socket, "message received", 20) < 0)
	{
		cout<<"writing socket error!"<<endl;
		exit(1);
	}

	close(new_socket);
	close(socket_server);
	return 0;
}