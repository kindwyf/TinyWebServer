#ifndef LST_TIMER
#define LST_TIMER

#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <sys/stat.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/uio.h>

#include <time.h>
#include "../log/log.h"

class util_timer;

// 用户数据结构
struct client_data
{
    // 客户端socket地址
    sockaddr_in address;
    // socket文件描述符
    int sockfd;
    // 定时器
    util_timer *timer;
};

// 定时器类
class util_timer
{
public:
    util_timer() : prev(NULL), next(NULL) {}

public:
    // time_t定义为一个有符号长整型，用于存储自1970年1月1日以来经过的秒数。
    time_t expire;
    // 定时触发函数，参数是用户数据
    void (* cb_func)(client_data *);
    // 用户数据
    client_data *user_data;
    util_timer *prev;
    util_timer *next;
};

// 定时器双向链表，根据超时时间expire升序排列
class sort_timer_lst
{
public:
    sort_timer_lst();
    ~sort_timer_lst();
    // 加入计时器（expire小于当前链表表头）
    void add_timer(util_timer *timer);
    // 调整计时器位置
    void adjust_timer(util_timer *timer);
    // 删除计时器
    void del_timer(util_timer *timer);
    // SIGALRM信号触发时就在其信号处理函数中执行tick，处理链表上到期的任务
    void tick();

private:
    // 将目标定时器timer插入lst_head之后的部分链表中
    void add_timer(util_timer *timer, util_timer *lst_head);

    util_timer *head;
    util_timer *tail;
};

class Utils
{
public:
    Utils() {}
    ~Utils() {}

    void init(int timeslot);

    //对文件描述符设置非阻塞
    int setnonblocking(int fd);

    //将内核事件表注册读事件，ET模式，选择开启EPOLLONESHOT
    void addfd(int epollfd, int fd, bool one_shot, int TRIGMode);

    //信号处理函数
    static void sig_handler(int sig);

    //设置信号函数
    void addsig(int sig, void(handler)(int), bool restart = true);

    //定时处理任务，重新定时以不断触发SIGALRM信号
    void timer_handler();

    void show_error(int connfd, const char *info);

public:
    static int *u_pipefd;
    sort_timer_lst m_timer_lst;
    static int u_epollfd;
    int m_TIMESLOT;
};

void cb_func(client_data *user_data);

#endif
