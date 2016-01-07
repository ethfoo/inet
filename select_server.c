#include <stdio.h>
#include <assert.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>

#define BUF_SIZE 100

int main()
{
	char *port = "2500";

	int ret = 0;
	int serv_sock;
	int clnt_sock;

	struct sockaddr_in clnt_addr;
	socklen_t clnt_addr_size;

	struct sockaddr_in serv_addr;
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(atoi(port));
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	fd_set read_set, cpy_read_set;
	fd_set write_set;
	FD_ZERO(&read_set); 
	FD_ZERO(&write_set); 
	int fd_max, fd_num, i, str_len;
	char buf[BUF_SIZE];

	serv_sock = socket(AF_INET, SOCK_STREAM, 0);
	assert( serv_sock >= 0 );
	printf("socket\n");

	ret = bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
	assert(ret != -1);
	printf("bind\n");

	ret = listen(serv_sock, 2);
	assert( ret != -1 );
	printf("listenning.., ret=%d\n", ret);

	FD_SET(serv_sock, &read_set);
	fd_max = serv_sock;

	while(1)
	{
		cpy_read_set = read_set;
		
		if( (fd_num = select(fd_max+1, &cpy_read_set, NULL, NULL, NULL)) == -1 )
		{
			break;
		}
		if( fd_num == 0 )
		{
			continue;
		}

		for( i = 0; i<=fd_max; i++)
		{
			if( FD_ISSET(i, &cpy_read_set))
			{
				if( i == serv_sock )
				{
					clnt_addr_size = sizeof(clnt_addr);
					clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
					printf("accepted\n");
					printf("connected client: %d \n", clnt_sock);
					assert( clnt_sock != -1 );
					FD_SET( clnt_sock, &read_set );
					if( fd_max < clnt_sock )
						fd_max = clnt_sock;
				}
				else
				{
					str_len = read(i, buf, BUF_SIZE);
					if(str_len == 0)
					{
						printf("received: %s\n", buf);

						FD_CLR( i, &read_set );
						close(i);
						printf("closed client: %d \n", i);
					}
					else
					{
						write(i, buf, str_len);
					}
					
				}
			}
		}

	}

	close(serv_sock);
	return 0;
}

