#include "ThreadPool.h"
#include <stdio.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

/**
 * define pool related
 */
threadpool_t *thp = NULL,*thpPSAM = NULL;
/**
 ******************************************************************************
 ** \简  述  mark thread alive or dead
 **  注  意
 ** \参  数  status:if alive ,then set status 
 ** \返回值   应用线程的Tid为！0值
 ** \作  者  
 ******************************************************************************/
void ThreadStatus(threadpool_t* pool,bool status)
{
	int i = 0;
	pthread_mutex_lock(&(pool->lock));
	for( i = 0;i<pool->live_thr_num;i++)
	{
		if((pool->threads[i] != 0x00)&&(pthread_self() == pool->threads[i]))
		{
			pool->ThreadStatus[i] = status;
		}
	}
	pthread_mutex_unlock(&(pool->lock));
}
/**
 ******************************************************************************
 ** \简  述  线程池中各个工作线程
 **  注  意  入栈的函数请不要写for(;;)/while(1)之类的死循环
 ** \参  数
 ** \返回值
 ** \作  者  
 ******************************************************************************/
void *threadpool_thread(void *threadpool)
{
	threadpool_t *pool = (threadpool_t*)threadpool;
	threadpool_task_t task;
	pthread_detach(pthread_self());
	ThreadStatus(threadpool,true);
	handle *curlhandle = NULL;
	curlhandle = (handle*)malloc(sizeof(handle));
	curlhandle->curlpost      = NULL;
	curlhandle->curlpostvalid = 0;
	curlhandle->handlethread  = 0;
	while(true)
	{
		pthread_mutex_lock( &(pool->lock) );
		while( (pool->queue_size ==0) && (!pool->shutdown) )
		{
			//printf("thread 0x%x is waiting \n",(unsigned int)pthread_self());
			pthread_cond_wait(&(pool->queue_not_empty), &(pool->lock));
			if( pool->wait_exit_thr_num > 0)
			{
				pool->wait_exit_thr_num--;
				if (pool->live_thr_num > pool->min_thr_num)
				{
					pool->live_thr_num--;
					pthread_mutex_unlock(&(pool->lock));
				//	printf("exit threadpool_thread threadID is 0x%x \n",(unsigned int)pthread_self());
					ThreadStatus(threadpool,false);
					if(curlhandle->curlpostvalid == 1){
						if(curlhandle->curlpost != NULL)
							curl_easy_cleanup(curlhandle->curlpost);
						curlhandle->curlpost = NULL;
						curlhandle->curlpostvalid = 0;
					}
					free(curlhandle);
					pthread_exit(0);
				}
			}
		}
		if(pool->shutdown)//exit all thread
		{
			if(curlhandle->curlpostvalid == 1){
				if(curlhandle->curlpost != NULL)
					curl_easy_cleanup(curlhandle->curlpost);
				curlhandle->curlpost = NULL;
				curlhandle->curlpostvalid = 0;
			}
			free(curlhandle);
			pthread_mutex_unlock ( &(pool->lock) );
			pthread_exit(0);
		}
		task.function=pool->task_queue[ pool->queue_front ].function;
		task.arg = pool->task_queue[ pool->queue_front ].arg;
		pool->queue_front = (pool->queue_front +1)%pool->queue_max_size;
		pool->queue_size--;

		pthread_cond_broadcast(&(pool->queue_not_full));
		pthread_mutex_unlock(&(pool->lock));
		//printf("thread 0x%x start working\n", (unsigned int)pthread_self());
		pthread_mutex_lock(&(pool->thread_counter));
		pool->busy_thr_num++;
		pthread_mutex_unlock(&(pool->thread_counter));
		(task.function)(task.arg,curlhandle);//run function
		pthread_mutex_lock(&(pool->thread_counter));
		pool->busy_thr_num--;
		pthread_mutex_unlock(&(pool->thread_counter));
	}
	if(curlhandle->curlpostvalid == 1){
        if(curlhandle->curlpost != NULL)
			curl_easy_cleanup(curlhandle->curlpost);
        curlhandle->curlpost = NULL;
        curlhandle->curlpostvalid = 0;
    }
	free(curlhandle);
	pthread_exit(0);
}
/**
 ******************************************************************************
 ** \简  述  adjust thread
 **  注  意  根据需要自动调整线程数量，API接口只需要设置最小值即可
 ** \参  数
 ** \返回值
 ** \作  者  RSU
 ******************************************************************************/
void *adjust_thread(void *threadpool)
{
	int i = 0,add = 0,ret = 0;
	pthread_attr_t attr_ThreadTrade;
	struct sched_param sched_param;
	threadpool_t *pool = (threadpool_t *)threadpool ;
	pthread_detach(pthread_self());
	//printf("adjust_thread threadID is %d\n", (unsigned int)pthread_self());
	while( !(pool->shutdown)  )
	{
		sleep(DEFAULT_TIME);
		pthread_mutex_lock(&(pool->lock));
		int queue_size = pool->queue_size;
		int live_thr_num = pool->live_thr_num;
		pthread_mutex_unlock(&(pool->lock));
		pthread_mutex_lock(&(pool->thread_counter));
		int busy_thr_num = pool->busy_thr_num;
		pthread_mutex_unlock(&(pool->thread_counter));
		/* 创建新线程 算法： 任务数大于最小线程池个数, 且存活的线程数少于最大线程个数时 如：30>=10 && 40<100*/
		if (queue_size >= pool->min_thr_num && live_thr_num < pool->max_thr_num)
		{
			pthread_mutex_lock(&(pool->lock));

			for (i = 0,add = 0; i < pool->max_thr_num && add < DEFAULT_THREAD_VARY&& pool->live_thr_num < pool->max_thr_num; i++)
			{
				if (pool->threads[i] == 0 || (pool->ThreadStatus[i] == false))
				{
					sched_param.sched_priority = pool->ThreadPriority[i];
					if(((ret = pthread_attr_init(&attr_ThreadTrade)) < 0)&&((ret = pthread_attr_setschedpolicy(&attr_ThreadTrade,SCHED_RR))<0)&&((ret = pthread_attr_setschedparam(&attr_ThreadTrade,&sched_param))<0))
					{
						printf("thread pool create attr_ThreadTrade fail return is %d\n",ret);
					}
					pthread_attr_setinheritsched(&attr_ThreadTrade,PTHREAD_EXPLICIT_SCHED);
					ret = pthread_attr_setstacksize(&attr_ThreadTrade,2*1024*1024 ); 
					pool->live_thr_num++;
					pthread_create(&(pool->threads[i]), &attr_ThreadTrade, threadpool_thread, (void *)pool);
					ret = pthread_attr_destroy(&attr_ThreadTrade);
					add++;
					//printf("live new number add is %d \n",pool->live_thr_num);
				}
			}
			pthread_mutex_unlock(&(pool->lock));
		}
		/* 销毁多余的空闲线程 算法：忙线程X2 小于 存活的线程数 且 存活的线程数 大于 最小线程数时*/
		if ((busy_thr_num * 2) < live_thr_num  &&  live_thr_num > pool->min_thr_num)
		{
			pthread_mutex_lock(&(pool->lock));
			pool->wait_exit_thr_num = DEFAULT_THREAD_VARY;
			pthread_mutex_unlock(&(pool->lock));
			for (i = 0; i < DEFAULT_THREAD_VARY; i++)
			{
				pthread_cond_signal(&(pool->queue_not_empty));
				usleep(5000);
			}
		}
	}
	return NULL;
}
/**
 ******************************************************************************
 ** \简  述  free threadpool source
 **  注  意
 ** \参  数
 ** \返回值
 ** \作  者  RSU
 ******************************************************************************/
bool threadpool_free(threadpool_t *pool)
{
	if (pool == NULL){
		return -1;}
	if (pool->task_queue){
		free(pool->task_queue);
	}
	if(pool->ThreadStatus)
	       free(pool->ThreadStatus);
	if(pool->ThreadPriority){
		free(pool->ThreadPriority);
	}
	if (pool->threads){
		free(pool->threads);
		pthread_mutex_unlock(&(pool->lock));
		pthread_mutex_destroy(&(pool->lock));
		pthread_mutex_unlock(&(pool->thread_counter));
		pthread_mutex_destroy(&(pool->thread_counter));
		pthread_cond_destroy(&(pool->queue_not_empty));
		pthread_cond_destroy(&(pool->queue_not_full));
	}
	free(pool);
	pool = NULL;
	return true;
}
/**
 ******************************************************************************
 ** \简  述  destory threadpool source
 **  注  意
 ** \参  数
 ** \返回值
 ** \作  者  RSU
 ******************************************************************************/
bool threadpool_destroy(threadpool_t *pool)
{
	int i = 0;
	if (pool == NULL)
	{
		return false;
	}
	pool->shutdown = true;
	for (i = 0; i < pool->live_thr_num; i++)
	{
		pthread_cond_broadcast(&(pool->queue_not_empty));
	}
	threadpool_free(pool);
	return true;
}
/**
 ******************************************************************************
 ** \简  述  add task to thread pool
 **  注  意
 ** \参  数
 ** \返回值
 ** \作  者  RSU
 ******************************************************************************/
bool threadpool_add(threadpool_t *pool,void (*function)(void* arg,void* curl),void *arg)
{
	pthread_mutex_lock(&(pool->lock));
	while((pool->queue_size == pool->queue_max_size)&&(!pool->shutdown))
	{
		/* 当任务队列满时，添加任务的线程阻塞，等待此条件变量 */
		//对于读数据库指令（可以下一次再读 反正不会丢失）与heart指令就算丢失也不会产生影响
		//对于正常数据指令，绝对不能够阻塞，在极端情况下容易丢失（虽然概略很低）
		//pthread_cond_wait(&(pool->queue_not_full),&(pool->lock));
		//printf("pthread_cond_wait aready full\n");
		pthread_mutex_unlock(&(pool->lock));
		return false;
	}
	if(pool->shutdown)
	{
		pthread_mutex_unlock(&(pool->lock));
	}
	if(pool->task_queue[pool->queue_rear].arg != NULL)
	{
		pool->task_queue[pool->queue_rear].arg = NULL;
	}
	pool->task_queue[pool->queue_rear].function = function;
	pool->task_queue[pool->queue_rear].arg = arg;
	pool->queue_rear = (pool->queue_rear + 1)%(pool->queue_max_size);
	pool->queue_size++;
	pthread_cond_signal(&(pool->queue_not_empty));
	pthread_mutex_unlock(&(pool->lock));
	return true;
}

/**
 ******************************************************************************
 ** \简  述  create thread pool
 **  注  意
 ** \参  数
 ** \返回值
 ** \作  者  RSU
 ******************************************************************************/
threadpool_t *threadpool_create(int min_thr_num, int max_thr_num, int queue_max_size)
{
	int i = 0,ret = 0;
	pthread_attr_t attr_ThreadTrade;
	struct sched_param sched_param;
	pthread_attr_t attr_Threadadjust;
	threadpool_t *pool = NULL;
	do
	{
		if((pool = (threadpool_t *)malloc(sizeof(threadpool_t))) == NULL)
		{
			//printf("alloc pool fail,please reboot\n");
			break;
		}
		memset(pool,0,sizeof(threadpool_t));
		pool->min_thr_num = min_thr_num;
		pool->max_thr_num = max_thr_num;
		pool->live_thr_num = min_thr_num;
		pool->queue_max_size = queue_max_size;
		pool->threads = (pthread_t *)malloc(sizeof(pthread_t)*max_thr_num);
		pool->ThreadPriority = (int*)malloc(sizeof(int)*max_thr_num);
		pool->task_queue = (threadpool_task_t *)malloc(sizeof(threadpool_task_t)*queue_max_size);
        pool->shutdown   = false;
		pool->ThreadStatus = (bool*)malloc(sizeof(bool)*max_thr_num);
		if ((pool->threads == NULL)||(pool->ThreadPriority == NULL)||(pool->ThreadStatus == NULL)||(pool->task_queue == NULL))
		{
			//printf("alloc thread pool fail,please reboot\n");
			break;
		}
		for (i = 0; i < pool->max_thr_num; i++)
		{
			pool->ThreadPriority[i] = 63 + i;
		}
 
		memset(pool->threads, 0, sizeof(pthread_t)*max_thr_num);
		memset(pool->ThreadStatus, 0, sizeof(bool)*max_thr_num);
		memset(pool->task_queue, 0, sizeof(threadpool_task_t)*queue_max_size);
		if (pthread_mutex_init(&(pool->lock), NULL) != 0|| pthread_mutex_init(&(pool->thread_counter), NULL) != 0
			|| pthread_cond_init(&(pool->queue_not_empty), NULL) != 0|| pthread_cond_init(&(pool->queue_not_full), NULL) != 0)
		{
			//printf("init the lock or cond fail,please reboot");
			break;
		}
		for (i = 0; i < min_thr_num; i++)
		{
		//	pthread_attr_init(&attr_ThreadTrade);
			sched_param.sched_priority = pool->ThreadPriority[i];
			if((pthread_attr_init(&attr_ThreadTrade) != 0)||(0 != pthread_attr_setschedpolicy(&attr_ThreadTrade,SCHED_RR))||(0 != pthread_attr_setschedparam(&attr_ThreadTrade,&sched_param)))
			{
				printf("thread pool create thread initialization error\n");
			}
			pthread_attr_setinheritsched(&attr_ThreadTrade,PTHREAD_EXPLICIT_SCHED);
			ret = pthread_attr_setstacksize(&attr_ThreadTrade,2*1024*1024 );//2M for adjust thread
			pthread_create(&(pool->threads[i]), &attr_ThreadTrade, threadpool_thread, (void *)pool);
			if(0x00 != pthread_attr_destroy(&attr_ThreadTrade))
			{
				printf("thread pool create thread the function pthread_attr_destroy return error\n");
			}
			//printf("start thread 0x%x...\n", (unsigned int)pool->threads[i]);
		}
		if((ret = pthread_attr_init(&attr_Threadadjust)) !=0)
			return NULL;
		ret = pthread_attr_setstacksize(&attr_Threadadjust, 20*1024);//32K for adjust thread
		pthread_create(&(pool->adjust_tid), &attr_Threadadjust, adjust_thread, (void *)pool);
		if(0x00 != pthread_attr_destroy(&attr_Threadadjust))
			printf("thread adjust create thread the function pthread_attr_destroy return error\n");
		return pool;

	} while (0);
	threadpool_free(pool);
	return NULL;
}
