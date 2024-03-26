#include <iostream>
#include <vector>   //向量存储
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
using namespace std;


#define    MAX_NUM   1024


class MyEpoll{
    public:
    MyEpoll();
    ~MyEpoll();

    int epoll_create();
    int epoll_add(int listenfd);
    int epoll_del(int listenfd);
    int epoll_mod(int listenfd);
    int myepoll_wait();
    struct epoll_event* getEvent();

    private:
    int epfd;
    int epoll_events_max_nums;
    struct epoll_event * events;   //就绪队列
};

MyEpoll::MyEpoll(){
    this->epfd = -1;
    this->epoll_events_max_nums = MAX_NUM;
    this->events = NULL;
}



MyEpoll::~MyEpoll(){
    if(epfd > 0){
        close(epfd);
    }

    if(events != NULL){
        delete[] this->events;
        this->events = NULL;
    }
}

int MyEpoll::epoll_create(){
    this->epfd = epoll_create(1);
    if(epfd == -1){
        printf("epoll_create error\n");
        return -1;
    }
    if(events != NULL){
        delete [] events;
    }

    events = new epoll_event[epoll_events_max_nums];
}


int MyEpoll::epoll_add(int listenfd){
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = listenfd;

    epoll_ctl(epfd,EPOLL_CTL_ADD,listenfd,&ev);
}

int MyEpoll::epoll_del(int listenfd){
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = listenfd;

    epoll_ctl(epfd,EPOLL_CTL_DEL,listenfd,&ev);
}

int MyEpoll::epoll_mod(int listenfd){
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = listenfd;

    epoll_ctl(epfd,EPOLL_CTL_MOD,listenfd,&ev);
}

int MyEpoll::myepoll_wait(){
    int nfds = epoll_wait(epfd,events,MAX_NUM,-1);
    return nfds;
}

struct epoll_event* MyEpoll::getEvent(){
    return events;
}

#define  SERV_PORT       9999
#define  BUFF_LENGTH     320

int main(){
    MyEpoll myepoll;
    int sockfd = socket(AF_INET,SOCK_STREAM,0);
	if(sockfd == -1){
		printf("socket error!\n");
		return -1;
	}
	
	struct sockaddr_in serv_addr;
	struct sockaddr_in clit_addr;
	socklen_t clit_len;
	
	
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(SERV_PORT);
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);// 任意本地ip
	
	int ret = bind(sockfd,(struct sockaddr*)&serv_addr,sizeof(serv_addr));
	if(ret == -1){
		printf("bind error!\n");
		return -2;
	}
	
	listen(sockfd,MAX_LINK_NUM);

    myepoll.epoll_create();
    myepoll.epoll_add(sockfd);
    int connfd = 0;
    int count = 0;
    for(;;){
        int nfds = myepoll.myepoll_wait();
        if(nfds == -1){
				printf("epoll_wait error!\n");
				return -5;
			}
			printf("nfds: %d\n",nfds);
    struct epoll_event *events = myepoll.getEvent();
    for(int i = 0;i<nfds;++i){
				//客服端有新的请求
				
				if(events[i].data.fd == sockfd){
					 connfd =  accept(sockfd,(struct sockaddr*)&clit_addr,&clit_len);
					if(connfd == -1){
						printf("accept error!\n");
						return -6;
					}
                    myepoll.epoll_add(connfd);
					
					printf("accept client: %s\n",inet_ntoa(clit_addr.sin_addr));
					printf("client %d\n",++count);

    }
    else{
					char buff[BUFF_LENGTH];
					int ret1 = read(connfd,buff,sizeof(buff));
					printf("%s",buff);
				}
    
}
    }
}
