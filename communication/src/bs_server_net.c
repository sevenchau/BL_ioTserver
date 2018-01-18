/**
  ******************************************************************************
  * @file    : bs_server_net.c 
  * @author  : Seven K. Zhou
  * @version : V 1.0.0
  * @date    : 2017/07/15
  ******************************************************************************
  * @brief  cummunicate by TCP/UDP socket
  ******************************************************************************
  */
#include "../inc/bs_server_net.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/epoll.h>
#include <errno.h>

#define EP_CREATE_SIZE                      5000
#define EVENTSIZE							100

#define MYPORT  							8080

#define QUEUE   							20
#define BUFFER_SIZE 						1024

/*******************************************************************************
 * @brief  socket_no_blocking(): set socket no blocking
 * @param  fd : the socket fd
 * @retval int32_t , 0: means successful  -1: means failed
 * Author: 2017/7/23, by Seven K. Zhou
*******************************************************************************/
static int32_t __socket_no_blocking(int fd)
{
	int flags = fcntl(fd, F_GETFL, 0);
	if (-1 == flags) {
		fprintf(stderr, ":> socket F_GETFL no-block setting fail! abort( )\n");
		return -1;
	}
	
	if (-1 == fcntl(fd, F_SETFL, flags|O_NONBLOCK)) {
		fprintf(stderr, ":> socket F_SETFL no-block setting fail! abort( )\n");
		return -1;
	}

	return 0;
}


/*******************************************************************************
 * @brief  socket_keep_alive(): set the socket keep alive.
 * @param  fd : the socket fd
 * @retval int32_t , 0: means successful  -1: means failed
 * Author: 2017/7/23, by Seven K. Zhou
 
 * keep alive 机制;及时有效地检测到一方的非正常断开
 * int keepalive = 1;	  // 开启keepalive属性
 * int keepidle = 60;	  // 如该连接在60秒内没有任何数据往来,则进行探测
 * int keepinterval = 5; // 探测时发包的时间间隔为5 秒
 * int keepcount = 3;	  // 探测尝试的次数.如果第1次探测包就收到响应了,则后2次的不再发.
 * setsockopt(rs, SOL_SOCKET, SO_KEEPALIVE, (void *)&keepalive , sizeof(keepalive));
 * setsockopt(rs, SOL_TCP, TCP_KEEPIDLE, (void*)&keepidle , sizeof(keepidle ));
 * setsockopt(rs, SOL_TCP, TCP_KEEPINTVL, (void *)&keepinterval , sizeof(keepinterval ));
 * setsockopt(rs, SOL_TCP, TCP_KEEPCNT, (void *)&keepcount , sizeof(keepcount ));
*******************************************************************************/
static int32_t __socket_keep_alive(int32_t infd) {
    int32_t keepalive = 1;
    int32_t keepcount = 1;
    int32_t keepinterval = 2;
    int32_t keepidle = 120;
    int32_t ret = 0;
	
    ret = setsockopt(infd, SOL_SOCKET, SO_KEEPALIVE, (void *)&keepalive, sizeof(keepalive));
    if (ret != 0) {
        fprintf(stderr, "failed to set socket option SO_KEEPALIVE: %s\n", strerror(errno));
        return -1;
    }
    
    ret = setsockopt(infd, IPPROTO_TCP, TCP_KEEPCNT, (void *)&keepcount, sizeof(keepcount));
    if (ret != 0) {
        fprintf(stderr, "failed to set tcp option TCP_KEEPCNT: %s\n", strerror(errno));
        return -1;
    }
    
    ret = setsockopt(infd, IPPROTO_TCP, TCP_KEEPIDLE, (void *)&keepidle, sizeof(keepidle));
    if (ret != 0) {
        fprintf(stderr, "failed to set tcp option TCP_KEEPIDLE: %s\n", strerror(errno));
        return -1;
    }
	
    ret = setsockopt(infd, IPPROTO_TCP, TCP_KEEPINTVL, (void *)&keepinterval, sizeof(keepinterval));
    if (ret != 0) {
        fprintf(stderr, "failed to set tcp option TCP_KEEPINTVL: %s\n", strerror(errno));
        return -1;
    }
    return 0;
}



void thread_pkt_downlink(void *arg)
{
	int32_t ret = 0;
	
    // 定义sockfd
    int sockfd = socket(AF_INET,SOCK_STREAM, 0);

    // 定义sockaddr_in
    struct sockaddr_in local_sockaddr;
    local_sockaddr.sin_family = AF_INET;
    local_sockaddr.sin_port = htons(MYPORT);
    local_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	fprintf(stdout, "%s id:%d\n", __FUNCTION__,(int32_t)arg);

	//一个端口释放后会等待两分钟之后才能再被使用，SO_REUSEADDR是让端口释放后立即就可以被再次使用。
	//SO_REUSEADDR用于对TCP套接字处于TIME_WAIT状态下的socket，允许重复绑定使用。
	//server程序总是应该在调用bind()之前设置SO_REUSEADDR套接字选项。
	int value = 1;
	if (-1 == setsockopt(sockfd, SOL_SOCKET,SO_REUSEADDR, &value, sizeof(value))) {
		fprintf(stderr, ":> setsockopt SO_REUSEADDR fail !!\r\n");
	}

    // bind，成功返回0，出错返回-1
    if(bind(sockfd, (struct sockaddr *)&local_sockaddr, sizeof(local_sockaddr)) == -1)
    {
        fprintf(stderr, ":> bind had binded failed, it will retry!!\r\n");
		
        return;
    }

	// 设置为非阻塞模式
	ret = __socket_no_blocking(sockfd);
	if (-1 == ret) {
		fprintf(stderr, ":> socket no blocking had set failed, abort().!! \r\n");
		abort();
	}

    // listen，成功返回0，出错返回-1
    if(listen(sockfd, SOMAXCONN) == -1)
    {
		fprintf(stderr, ":> listen fail! abort( )\n");
		abort();
    }

	// 非阻塞模式下把I/O事件交给对象select、epoll等处理
    // receive packets from bs by epoll events
    int32_t ep_fd = epoll_create(EP_CREATE_SIZE);
	if (-1 == ep_fd) {
        fprintf(stderr, ":> epoll_create error ! abort( )\n");
        abort();
    }
    struct epoll_event event;
    struct epoll_event events[EVENTSIZE];
    event.data.fd  = sockfd;
    event.events = EPOLLIN|EPOLLET;
    if (-1 == epoll_ctl(ep_fd, EPOLL_CTL_ADD, sockfd, &event)) {
        fprintf(stderr, ":> epoll_ctl error ! abort( )\n");
        abort();
    }

    char rd_buff[BUFFER_SIZE];
	int32_t rd_len = 0;
	
    // 客户端套接字	
    struct sockaddr_in client_addr;
    socklen_t length = sizeof(client_addr);

	int32_t event_num = 0; // 要处理事件的数目
	int32_t i = 0;

	int conn_fd;
	
	fprintf(stdout, "%s create successful\r\n", __FUNCTION__);
	
    while(1)
    {
        // 2.6.17 版本内核中增加了 EPOLLRDHUP 事件，代表对端断开连接
        // 第4个参数：-1相当于阻塞，0相当于非阻塞。一般用-1即可。
        event_num = epoll_wait(ep_fd, events, EVENTSIZE, -1);
		fprintf(stdout, "epoll_wait events number:%d\r\n", event_num);
		for (i=0; i<event_num; i++) {
			 if ((events[i].events & EPOLLIN)) { //如果是已经连接的用户，并且收到数据，那么进行读入。
				  if ((events[i].data.fd)< 0) {
					  continue;
				  }
				//如果新监测到一个SOCKET用户连接到了绑定的SOCKET端口，则建立新的连接。
				// 并且设置为keep alive机制， 设置为非阻塞模式。
				 if(sockfd == events[i].data.fd) {
					 fprintf(stderr, "the new socket ...\r\n");
					 //建立新的连接。conn_fd 为新的socket
					 conn_fd = accept(sockfd, (struct sockaddr*)&client_addr, &length);
					 if (conn_fd<0) {
						 fprintf(stderr, ":> connect error ! abort( )\n");
						 abort();
					 }

					 // keep alive 机制;及时有效地检测到一方的非正常断开
					 ret = __socket_keep_alive(conn_fd);
					 if (-1 == ret) {
						fprintf(stderr, ":> socket keep alive fail ! close socket.\n");
						close(conn_fd);
						break;
					 }

					 // 设置为非阻塞模式
					 ret = __socket_no_blocking(conn_fd);
					 if (-1 == ret) {
						 fprintf(stderr, ":> socket no blocking had set failed, abort().!! ");
						 abort();
					 }

					 event.data.fd	 = conn_fd;
					 event.events = EPOLLIN | EPOLLET;
					 
					 // 注册event到epoll中
					 ret = epoll_ctl(ep_fd, EPOLL_CTL_ADD, conn_fd, &event);
					 if (ret == -1) {
						 fprintf(stderr, "epoll_ctl error!!\n");
						 close(conn_fd);
					 }

					 // TOTO: 将conn_fd添加到链表中，以备后续查询并使用。需要建立一个FD_LIST
					 
				 }
				 else {
					 // 读取数据直到读空
					 while(1) {
						fprintf(stderr, "receive message->");
						memset(rd_buff, 0, sizeof(rd_buff));
						rd_len = read(events[i].data.fd, rd_buff, sizeof(rd_buff));
						 
						 if (rd_len < 0 ) { // 数据错误
							 fprintf(stderr, "error\r\n");
							 if(errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
								 break;
							 }
							 close(events[i].data.fd);
							 fprintf(stderr, "close fd:rd_len < 0\r\n");
							 //if socket err, delete from list and close(fd)
							 // TDOO: ret = TDOO(events[i].data.fd);
							 if (ret != 0) {
								 fprintf(stderr, ":> delete and close failed !!\n");
								 abort();
							 }
							 break;
					 
						 }
						 else if (0 == rd_len) { // 无数据
							 fprintf(stderr, "no message\r\n");
							 // TDOO: ret = TDOO(events[i].data.fd);
							 close(events[i].data.fd);
							 fprintf(stderr, "close fd:rd_len == 0\r\n");
							 if (ret != 0) {
								 fprintf(stderr, ":> delete and close failed!!\n");
								 abort();
							 }
							 break;
						 }
						 else { // 有数据
						 /* TODO:处理接收到的数据，如入链表等操作 */
						 //    (events[i].data.fd, rd_buff, rd_len); 
							fprintf(stdout, "has message\r\n");
							fprintf(stdout, "%s\r\n", rd_buff);
						 }
					 }
				}
			 }
			 else if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP) ||
				 (events[i].events & EPOLLRDHUP)) {
				 
				 fprintf(stderr, "epoll error\n");
				 if (sockfd == events[i].data.fd) {
					 fprintf(stderr, "must re-bind now........\n");
					 abort();
				 }
				 else {
					 // TODO: ret = delete_and_close(events[i].data.fd);
					 close(events[i].data.fd);
					 fprintf(stderr, "close fd:event error\r\n");
					 if (ret != 0) {
						 fprintf(stderr, "delete_and_close:%s:%d\n", __func__, __LINE__);
						 abort();
					 }
				 }
				 continue;
			 }

		}
		usleep(10*1000);
    }
	
    close(conn_fd);
    close(sockfd);
	
    return ;
}

