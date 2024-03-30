#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#define NEED_EXIT_STATUS (1)

void *thread_func(void *arg)
{
    printf("thread started\n");
    int *value = (int *)arg;
    // 输出为 10
    printf("received value: %d\n", *value);
    // 修改传入的值
    *value = 100;
    sleep(1);

#if NEED_EXIT_STATUS
    // 需要返回结束状态码, 以下两种方法都可以，推荐使用 return
    // pthread_exit((void *)value);
    return value;
#else
    // 不需要返回结束状态码，以下两种方法都可以，推荐使用 return
    // pthread_exit(NULL);
    return NULL;
#endif
}

int main()
{
    pthread_t tid;
    int value = 10;
    // 创建一个新线程，并以 thread_func 作为入口函数，传递给 thread_func 函数的参数为 value
    int ret = pthread_create(&tid, NULL, thread_func, (void *)&value);
    if (ret != 0) {
        fprintf(stderr, "pthread_create error: %d\n", ret);
        return -1;
    }
    printf("created new thread with id %ld\n", tid);

#if NEED_EXIT_STATUS
    // 等待线程结束并获取退出状态
    void *thread_retval;
    ret = pthread_join(tid, &thread_retval);
    if (ret != 0) {
        fprintf(stderr, "pthread_join error: %d\n", ret);
        return -1;
    }

    int *retval = (int *)thread_retval;
    printf("thread exited with value: %d\n", *retval);
#else
    // 等待线程结束不获取退出状态
    ret = pthread_join(tid, NULL);
    if (ret != 0) {
        fprintf(stderr, "pthread_join error: %d\n", ret);
        return -1;
    }
    printf("thread %ld exit normal\n", tid);
#endif

    return 0;
}
