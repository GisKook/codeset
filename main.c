#include <sys/epoll.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "cndef.h"
#include "kfifo.h"

#define MAX_EVENT 64 
#define MAX_ACCEPTSOCKETS 1024
#define MAX_RECVLEN 4096

int main(){ 
	int efd = epoll_create1(O_CLOEXEC);
	if(efd == -1){
		fprintf(stderr, "create epoll error. %s %d\n", __FILE__,__LINE__);

		return -1;
	}

	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = STDIN_FILENO;
//	if(-1 == epoll_ctl(efd, EPOLL_CTL_ADD, STDIN_FILENO, &ev)){
//		fprintf(stderr, "add to epoll error. %s %d\n", __FILE__,__LINE__);
//		
//		return -1;
//	}
	
	int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(listen_fd == -1){
		fprintf(stderr, "create listen socket error. %s %d\n", __FILE__,__LINE__);
		close(efd);

		return -1;
	}
	struct sockaddr_in listen_addr;
	memset((void*)&listen_addr, 0, sizeof(listen_addr));
	listen_addr.sin_family = AF_INET;
	listen_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	listen_addr.sin_port = htons(40000);
	if(bind(listen_fd, (struct sockaddr*)&listen_addr, sizeof(struct sockaddr_in)) == -1){
		fprintf(stderr, "bind listen socket error. %s %d\n", __FILE__,__LINE__);
		close(listen_fd);
		close(efd);

		return -1;
	}

	if( -1 == listen(listen_fd, MAX_ACCEPTSOCKETS)){
		fprintf(stderr, "listen error. %s %d\n", __FILE__,__LINE__);

		return -1;
	}
	
	ev.events = EPOLLIN;
	ev.data.fd = listen_fd;
	if(-1 == epoll_ctl(efd, EPOLL_CTL_ADD, listen_fd, &ev)){
		fprintf(stderr, "add listen fd to epoll error. %s %d\n",__FILE__,__LINE__);
		close(listen_fd);
		close(efd);

		return -1;
	}

	struct epoll_event events[MAX_EVENT];
	int nfds = 0;
	int i;
	int conn_fd;
	char buf[MAX_RECVLEN];
	int len;

	
	for(;;){ 
		nfds = epoll_wait(efd, events, MAX_EVENT, -1);

		for(i = 0; i< nfds; ++i){
			if(events[i].data.fd == listen_fd){ 
				conn_fd = accept(listen_fd, NULL, NULL);
				if(unlikely(conn_fd == -1)){
					fprintf(stderr, "conn socket error. %s %d\n", __FILE__,__LINE__);
					continue;
				}
				fcntl(conn_fd, F_SETFD, fcntl(conn_fd, F_GETFD, 0)|O_NONBLOCK);
				ev.events = EPOLLIN | EPOLLET;
				ev.data.fd = conn_fd;
				if(unlikely(epoll_ctl(efd, EPOLL_CTL_ADD, conn_fd, &ev) == -1)){
					fprintf(stderr, "connected socket add to epoll error. %s %d\n", __FILE__,__LINE__);
					
					close(conn_fd);
				}
			}else if(events[i].data.fd == STDIN_FILENO){ 
				printf("this is stdin\n");
			}else{ 
				len = read(events[i].data.fd, buf, MAX_RECVLEN);
				buf[len] = 0;
				if(len < 0){
					assert(0);
				}
			}
		}
	}

	return 0;
}
