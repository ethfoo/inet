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

static int pipefd[2];

typedef struct epoll_event epoll_events;

int setnonblocking(int fd)
{
	int old_option = fcntl(fd, F_GETFL);
	int new_option = old_option | O_NONBLOCK;
	fcntl(fd, F_SETFL, new_option);
	return old_option;
}

void addfd( int epollfd, int fd)
{
	epoll_event event;
	event.data.fd = fd;
	event.events = EPOLLIN | EPOLLET;
	epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
	setnonblocking(fd);
}

void sig_handler( int sig)
{
	int save_errno = errno;
	int msg = sig;
	send( pipefd[1], (char*)&msg, 1, 0);
	errno = save_errno;
}

void addsig( int sig)
{
	struct sigaction sa;
	memset( &sa, '\0', sizeof(sa));
	sa.sa_handler = sig_handler;
	sa.sa_flags |= SA_RESTART;
	sigfillset( &sa.sa_mask );
	assert( sigaction(sig, &sa, NULL) != -1 );
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

	int listenfd = socket(AF_INET, SOCK_STREAM, 0);
	assert(listenfd>=0);

	ret = bind( listenfd, (struct sockaddr*)&address, sizeof(address));
	assert( ret != -1 );

	ret = listen( listenfd, 5 );
	assert( ret != -1 );

	epoll_events events[MAX_EVENT_NUMER];
	int epollfd = epoll_create(5);
	assert(epollfd != -1);

	addfd( epollfd, listenfd);

	/**/
	ret = socketpair(AF_UNIX, SOCK_STREAM, 0, pipefd);
	assert( ret != -1 );
	setnonblocking( pipefd[1]);
	addfd( epollfd, pipefd[0]);


	addsig( SIGHUP );
	addsig( SIGCHLD );
	addsig( SIGTERM );
	addsig( SIGINT );
	bool stop_server = false;
	
	while( !stop_server )
	{
		int number = epoll_wait( epollfd, events, MAX_EVENT_NUMER, -1 );
		if( (number<0) && (errno != EINTR)) 
		{
			printf("epoll failure\n");
			break;
		}

		for( int i=0; i<number; i++)
		{
			int sockfd = events[i].data.fd;
			if( sockfd == listenfd )
			{
				struct sockaddr_in client_address;
				socklen_t client_addrlen = sizeof( client_addrlen);
				int connfd = accept( listenfd, (struct sockaddr*)
						&client_address, &client_addrlen);
				addfd( epollfd, connfd );
			}
			else if( (sockfd == pipefd[0]) && (events[i].events & EPOLLIN))
			{
				int sig;
				char signals[1024];
				ret = recv( pipefd[0], signals, sizeof(signals), 0);
				if( ret == -1 || ret == 0 )
				{
					continue;
				}
				else
				{
					for(int i=0; i<ret; i++)
					{
						switch( signals[i])
						{
							case SIGCHLD:
							case SIGHUP:
								{
									continue;
								}
							case SIGTERM:
							case SIGINT:
								{
									printf("recv SIGINT\n");
									stop_server = true;
								}
						}
					}
				}
			}
		}

	}

	close(listenfd);
	close(epollfd);
	close(pipefd[0]);
	close(pipefd[1]);

	return 0;
}
