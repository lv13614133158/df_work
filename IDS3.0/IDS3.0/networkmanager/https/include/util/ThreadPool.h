#ifndef _THREAD_POOL_H
#define _THREAD_POOL_H
#ifdef __cplusplus
extern "C"{
#endif
#include <stdint.h>
#include <semaphore.h>
#include <pthread.h>
#include <stdbool.h>
#include <curl/curl.h>
/**
 ******************************************************************************
 ** \简  述  thread pool related
 **  注  意
 ** \返回值
 ** \作  者
 ******************************************************************************/
#define DEFAULT_TIME			 1//1 second check
#define MAX_WAIT_TASK_NUM 		 3 
#define MAX_TASK_LIST_NUM        3
#define DEFAULT_THREAD_VARY      2//every time to create thread
#define MINTHREADNUMBER  1        //最小线程池线程数量
#define QUEUEMAXSIZE     20      //任务队列上限100


typedef struct
{
	void (*function)(void*,void*);
	void *arg;
}threadpool_task_t;
//curl related
typedef struct __handle{
	CURL    *curlpost;
	unsigned char curlpostvalid;
	unsigned char handlethread;//标记该句柄是属于主线程还是处理线程，防止跨线程调度 主线程:1   处理线程：0 
}handle;

typedef struct threadpool_t {
	pthread_mutex_t lock;               /* 用于锁住本结构体 ，和条件变量一起使用 */
	pthread_mutex_t thread_counter;     /* 记录忙状态线程个数的锁 -- busy_thr_num */
	pthread_cond_t queue_not_full;      /* 当任务队列满时，添加任务的线程阻塞，等待此条件变量 */
	pthread_cond_t queue_not_empty;     /* 任务队列里不为空时，通知线程池中等待任务的线程 */

	pthread_t *threads;                 /* 存放线程池中每个线程的tid。数组 */
	pthread_t adjust_tid;               /* 存管理线程tid */
	threadpool_task_t *task_queue;      /* 任务队列 */

	bool *ThreadStatus;                   /*检测线程是否退出*/
	int *ThreadPriority;                /*线程优先级*/

	char min_thr_num;                    /* 线程池最小线程数 */
	char max_thr_num;                    /* 线程池最大线程数 */

	char live_thr_num;                   /* 当前存活线程个数 */
	char busy_thr_num;                   /* 忙状态线程个数 */
	int wait_exit_thr_num;              /* 要销毁的线程个数 */
	int queue_front;                    /* task_queue队头下标 */
	int queue_rear;                     /* task_queue队尾下标 */
	int queue_size;                     /* task_queue队中实际任务数 */
	int queue_max_size;                 /* task_queue队列可容纳任务数上限 */
	bool shutdown;                       /* 标志位，线程池使用状态，true或false */

}threadpool_t;
extern threadpool_t *consumerPtr,*producerPtr;

void ThreadStatus(threadpool_t* pool,bool status);
void *threadpool_thread(void *threadpool);
void *adjust_thread(void *threadpool);
bool threadpool_free(threadpool_t *pool);
bool threadpool_destroy(threadpool_t *pool);
bool threadpool_add(threadpool_t *pool,void (*function)(void* arg,void* curl),void *arg);
threadpool_t *threadpool_create(int min_thr_num, int max_thr_num, int queue_max_size);
#ifdef __cplusplus
}
#endif
#endif
