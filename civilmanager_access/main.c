#include <sys/epoll.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <unistd.h>
#include "cndef.h"
#include "sockets_buffer.h"
#include "cnconsole.h"
#include "processappdata.h"
#include "dblogin.h"
#include "loginmanager.h"
#include "cnconfig.h"
#include "dbcardinfo.h"
#include "toolkit.h"

#define MAX_EVENT 64 
#define MAX_ACCEPTSOCKETS 1024
#define MAX_RECVLEN 4096

int main(){
	if( 0 != cnconfig_loadfile("./conf.json")){
		fprintf(stderr, "config file is not load well.\n");
		return -1;
	}
	const char * dump = cnconfig_getvalue(DUMP);
	if(strlen(dump) == sizeof("true")-1 && 0 == strcmp("true", dump)){ 
		FILE *errfd = fopen("./stderr.log", "w");
		if(errfd == NULL){
			fprintf(stderr, "create stderr.log fail. error message %s\n", strerror(errno));
		}
		dup2(fileno(errfd), STDERR_FILENO);
		fclose(errfd);
		FILE *outfd = fopen("./stdout.log","w");
		if(errfd == 0){
			fprintf(stderr, "create stdout.log fail. error message %s\n", strerror(errno));
		}
		dup2(fileno(outfd), STDOUT_FILENO);
		fclose(outfd);
//		FILE *errfd = freopen("./stderr.log", "w", stderr);
//		if (errfd == NULL){
//			fprintf(stderr, "create stderr.log error. error message: %s", strerror(errno));
//		}
//		FILE *outfd = freopen("./stdout.log", "w", stdout);
//		if(outfd == NULL){
//			fprintf(stderr, "create stdout.log error. error message: %s", strerror(errno));
//		}
	}

	int efd = epoll_create1(O_CLOEXEC);
	if(efd == -1){
		fprintf(stderr, "create epoll error. %s %d\n", __FILE__,__LINE__);

		return -1;
	}

	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = STDIN_FILENO;
	if(-1 == epoll_ctl(efd, EPOLL_CTL_ADD, STDIN_FILENO, &ev)){
		fprintf(stderr, "add to epoll error. %s %d\n", __FILE__,__LINE__);

		return -1;
	}

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
	const char * szport = cnconfig_getvalue(BINDPORT);
	int port = atoi(szport);
	listen_addr.sin_port = htons(port);
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
	unsigned char buf[MAX_RECVLEN];
	int len;

	int fdsig[2];
	if(pipe2(fdsig,O_CLOEXEC) == -1){
		fprintf(stderr,"create pipe error.%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
		close(listen_fd);
		close(efd);

		return -1;
	}
	char fdbuf[16]; 
	int buflen;

	struct sockets_buffer* socket_buf = sockets_buffer_create(MAX_ACCEPTSOCKETS);
	assert(socket_buf != NULL);
	struct loginmanager * loginmanager = loginmanager_create();
	assert(loginmanager != NULL);
	struct processappdata * pad = processappdata_create(socket_buf, loginmanager, fdsig[0]);
	assert(pad != NULL);
	struct dblogin * dblogin = dblogin_start(loginmanager);
	assert(dblogin != NULL);

	char printrecvmessage = 0;

	for(;;){
		nfds = epoll_wait(efd, events, MAX_EVENT, -1);

		for(i = 0; i< nfds; ++i){
			if( (events[i].events & EPOLLERR) ||
					(events[i].events & EPOLLHUP) || 
					!(events[i].events & EPOLLIN)){
				fprintf(stderr, "epoll error. error messsage: %s events[i].events :%d\n", strerror(errno),events[i].events);
				close(events[i].data.fd);
				sockets_buffer_del(socket_buf, events[i].data.fd);
				processappdata_delete(pad, events[i].data.fd);

				continue;
			}else if(events[i].data.fd == listen_fd){ 
				conn_fd = accept(listen_fd, NULL, NULL);
				if(unlikely(conn_fd == -1)){
					fprintf(stderr, "conn socket error. %s %s %d\n", __FILE__,__FUNCTION__,__LINE__);
					continue;
				}
				fcntl(conn_fd, F_SETFL, (fcntl(conn_fd, F_GETFL)|O_NONBLOCK));
				ev.events = EPOLLIN | EPOLLET;
				ev.data.fd = conn_fd;
				if(unlikely(epoll_ctl(efd, EPOLL_CTL_ADD, conn_fd, &ev) == -1)){
					fprintf(stderr, "connected socket add to epoll error. %s %s %d\n", __FILE__,__FUNCTION__,__LINE__);

					close(conn_fd);
				}
			}else if(events[i].data.fd == STDIN_FILENO){ 
				len = read(STDIN_FILENO, buf, MAX_RECVLEN);
				int cmd = console_parsecmd(buf, socket_buf);
				switch(cmd){
					case QUIT:
						{
							write(fdsig[1], "1*", 2);
							processappdata_join(pad);
							close(listen_fd);
							close(efd);

							goto exit_flag;
						}
						break;
					case PRM:
						printrecvmessage = 1;
						break;
					case UNPRM:
						printrecvmessage = 0;
						break;
					default:
						break;
				}
			}else{ 
				int signal = 0;
				int tempfd = 0;
				for(;;){
					len = read(events[i].data.fd, buf, MAX_RECVLEN);
					if(len == -1){ 
						if(errno != EAGAIN){
							perror("read");
							printf ("Closed connection on descriptor %d\n", events[i].data.fd);

							// Closing the descriptor will make epoll remove it
							// from the set of descriptors which are monitored.
							tempfd = events[i].data.fd;
							close(events[i].data.fd); 
							signal = 1;
						}
						break;
					}else if(len == 0){
						fprintf (stderr,"Closed connection on descriptor %d\n", events[i].data.fd);

						// Closing the descriptor will make epoll remove it
						// from the set of descriptors which are monitored.
						tempfd = events[i].data.fd;
						close (events[i].data.fd);
						signal = 1;
						break;
					}

					buf[len] = 0;

					if(printrecvmessage == 1){
						toolkit_printbytes(buf, len);
					}
					sockets_buffer_add(socket_buf, events[i].data.fd,(char*)"192.168.1.1",  buf, len);
					signal = 0;
				}
				if(signal == 0){
					memset(fdbuf, 0, 16);
					buflen = sprintf(fdbuf, "%d*",events[i].data.fd);
					if(-1 == write(fdsig[1], fdbuf, buflen)){
						fprintf(stderr, "write pipe error.%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
					}
				}else if(signal == 1){ 
					sockets_buffer_del(socket_buf, tempfd);
					processappdata_delete(pad, tempfd);
				}
			}
		}
	}
exit_flag:
	sockets_buffer_destroy(socket_buf);
	processappdata_destroy(pad);
	loginmanager_destroy(loginmanager);
	dblogin_end(dblogin);

	return 0;
}
