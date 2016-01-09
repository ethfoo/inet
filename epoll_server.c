#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <assert.h>

#define EPOLL_SIZE 50
#define BUF_SIZE 100
int main()
{

	int serv_sock, client_sock;
	int ret;
	socklen_t addr_size;

	struct sockaddr_in serv_addr, client_addr;
	memset(&serv_addr, 0, sizeof(serv_addr) );
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(2500);
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	serv_sock = socket(AF_INET, SOCK_STREAM, 0);
	assert( serv_sock != -1 );

	ret = bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
	assert(ret != -1);

	ret = listen(serv_sock, 5);
	assert( ret != -1 );

	int epfd;
	int epcnt;
	struct epoll_event event;
	struct epoll_event *events;
	events = malloc(sizeof(struct epoll_event)*EPOLL_SIZE); 
	assert( events != NULL );

	epfd = epoll_create(EPOLL_SIZE);

	event.events = EPOLLIN;
	event.data.fd = serv_sock;
	ret = epoll_ctl(epfd, EPOLL_CTL_ADD, serv_sock, &event);
	assert( ret != -1 );
	
	int str_len;
	char buf[BUF_SIZE]; 

	while(1)
	{

		epcnt = epoll_wait(epfd, events, EPOLL_SIZE, -1);
		if( epcnt == -1 )
		{
			break;
		}
		int i;
		for( i=0; i<epcnt; i++)
		{
			if( events[i].data.fd == serv_sock )
			{
				addr_size = sizeof(client_addr);
				client_sock = accept(serv_sock, (struct sockaddr*)&client_addr, &addr_size);
				assert(client_sock != -1);
				
				event.events = EPOLLIN;
				event.data.fd = client_sock;
				epoll_ctl(epfd, EPOLL_CTL_ADD, client_sock, &event);
				printf("connected client:%d \n", client_sock);
			}
			else if(events[i].events & EPOLLIN) 
			{
				str_len = read(events[i].data.fd, buf, BUF_SIZE);
				//读取完
				if( str_len == 0 )
				{
					printf("receive: %s\n", buf);
					epoll_ctl(epfd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
					close(events[i].data.fd);
					printf("closed client: %d\n", events[i].data.fd);
				}
				else
				{
					write(events[i].data.fd, buf, str_len);
				}
			}
		}

	}
	close(serv_sock);
	close(epfd);
	return 0;
}
