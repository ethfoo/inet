#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <assert.h>
#include <errno.h>
#include <signal.h>

#define EPOLL_SIZE 50
#define BUF_SIZE 10

int sigpipe_ignore()
{
	struct sigaction sa;
	memset(&sa, 0, sizeof(struct sigaction));
	sa.sa_handler = SIG_IGN;
	if( sigaction(SIGPIPE, &sa, NULL ))
	{
		perror("sigaction error\n");
		return -1;
	}
	return 0;
}

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

	sigpipe_ignore();

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
	memset(buf, '\0', BUF_SIZE); 
	while(1)
	{

		epcnt = epoll_wait(epfd, events, EPOLL_SIZE, -1);
		if( epcnt == -1 )
		{
			perror("epoll_wait error\n");
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
				printf("event trigger once\n");
				memset(buf, '\0', BUF_SIZE); 
				str_len = read(events[i].data.fd, buf, BUF_SIZE-1);
				printf("receive: %s\n", buf);
				if( str_len <= 0 )
				{
					epoll_ctl(epfd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
					close(events[i].data.fd);
					//perror("str_len==0\n");
					printf("closed client: %d\n", events[i].data.fd);
				}
				else
				{
					//?write会中断循环
					write(events[i].data.fd, buf, str_len);
				}
			}
		}

	}
	close(serv_sock);
	close(epfd);
	return 0;
}
