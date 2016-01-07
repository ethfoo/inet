#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>


void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}

int main(int argc, char *argv[])
{
	char *port = "2500";

	int serv_sock;
	int clnt_sock;

	struct sockaddr_in serv_addr;
	struct sockaddr_in clnt_addr;
	socklen_t clnt_addr_size;

	char *message = "hello world";

	printf("creating socket\n");
	serv_sock = socket(AF_INET, SOCK_STREAM, 0);
	if(serv_sock == -1)
		error_handling("socket() error");

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(atoi(port));
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	printf("binding..\n");
	if(bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
		error_handling("bind() error");

	printf("listening..\n");
	if(listen(serv_sock, 5) == -1)
		error_handling("listen() error");

	clnt_addr_size = sizeof(clnt_addr);

	printf("ready to accept..\n");
	clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
	printf("had accepted\n");
	if(clnt_sock == -1)
		error_handling("accept() error");

	char receiv[30];
	memset( receiv, '\0', 30);
	if( read(clnt_sock, receiv, sizeof(receiv)-1) == -1)
		error_handling("read error");

	printf("%s\n", receiv);

//	write(clnt_sock, message, sizeof(message));
	close(clnt_sock);
	close(serv_sock);
	printf("closed\n");

	return 0;
}
