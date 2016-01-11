#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <pthread.h>
#include <signal.h>

#define MAX_EVENT_NUMER 1024
#define BUFFER_SIZE 10

typedef struct epoll_event epoll_events;

int setnonblocking(int fd)
{
	int old_option = fcntl(fd, F_GETFL);
	int new_option = old_option | O_NONBLOCK;
	fcntl(fd, F_SETFL, new_option);
	return old_option;
}

void addfd( int epollfd, int fd, bool enable_et)
{
	epoll_event event;
	event.data.fd = fd;
	event.events = EPOLLIN;
	if( enable_et )
	{
		event.events |= EPOLLET;
	}
	epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
	setnonblocking(fd);
}

int ignore_sigpipe()
{
	struct sigaction sa;
	memset(&sa, 0, sizeof(struct sigaction));
	sa.sa_handler = SIG_IGN;
	if( sigaction(SIGPIPE, &sa, NULL))
	{
		perror("sigaction error");
		return -1;
	}
	return 0;
}

void lt(epoll_event * events, int number, int epollfd, int listenfd)
{
	char buf[BUFFER_SIZE];
	for(int i=0; i<number; i++)
	{
		int sockfd = events[i].data.fd;
		if( sockfd == listenfd )
		{
			struct sockaddr_in client_address;
			socklen_t client_addrlength = sizeof( client_address );
			int connfd = accept( listenfd, (struct sockaddr*)&client_address, &client_addrlength);
			addfd(epollfd, connfd, false);
		}
		else if( events[i].events & EPOLLIN )
		{
			printf("event trigger once\n");
			memset(buf, '\0', BUFFER_SIZE);
			int ret = recv( sockfd, buf, BUFFER_SIZE-1, 0);
			if( ret <= 0 )
			{
				close(sockfd);
				printf("close client: %d\n", sockfd);
				continue;
			}
			//test write
			write(sockfd, buf, ret);

			printf("get %d bytes of content: %s\n", ret, buf);
		}
		else
		{
			printf("something else happened\n");
		}
	}
}

void et(epoll_event *events, int number, int epollfd, int listenfd)
{
	char buf[BUFFER_SIZE];
	for( int i=0; i<number; i++)
	{
		int sockfd = events[i].data.fd;
		if( sockfd == listenfd )
		{
			struct sockaddr_in client_address;
			socklen_t client_addrlength = sizeof(client_address);
			int connfd = accept(listenfd, (struct sockaddr*)&client_address, &client_addrlength);
			addfd(epollfd, connfd, true);
		}
		else if( events[i].events & EPOLLIN)
		{
			printf("event trigger once\n");
			while(1)
			{
				memset(buf, '\0', BUFFER_SIZE);
				int ret = recv( sockfd, buf, BUFFER_SIZE-1, 0);
				if( ret<0 )
				{
					if( (errno == EAGAIN ) || (errno == EWOULDBLOCK))
					{
						printf("read later\n");
						break;
					}
					printf("ret<0, close client: %d\n", sockfd);
					close(sockfd);
					break;
				}
				else if( ret == 0 )
				{
					printf("ret==0, close client: %d\n", sockfd);
					close(sockfd);
				}
				else
				{
					write(sockfd, buf, ret);
					printf("get %d bytes of content: %s\n", ret, buf);
				}
			}
		}
		else
		{
			printf("something else happened \n");
		}
	}
}

int main()
{
	int port = atoi("2500");

	int ret = 0;
	struct sockaddr_in address;
	//bzero(&address, sizeof(address));
	memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_port = htons(port);
	address.sin_addr.s_addr = htonl(INADDR_ANY);

	ignore_sigpipe();

	int listenfd = socket(AF_INET, SOCK_STREAM, 0);
	assert(listenfd>=0);

	ret = bind( listenfd, (struct sockaddr*)&address, sizeof(address));
	assert( ret != -1 );

	ret = listen( listenfd, 5 );
	assert( ret != -1 );

	epoll_events events[MAX_EVENT_NUMER];
	int epollfd = epoll_create(5);
	assert(epollfd != -1);

	addfd( epollfd, listenfd, true );
	
	while(1)
	{
		int ret = epoll_wait( epollfd, events, MAX_EVENT_NUMER, -1 );
		if(ret<0)
		{
			printf("epoll failure\n");
			break;
		}
		lt( events, ret, epollfd, listenfd );
		//et( events, ret, epollfd, listenfd );

	}

	close(listenfd);
	close(epollfd);

	return 0;
}
