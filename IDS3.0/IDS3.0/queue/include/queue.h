//ids队列  队列满  停止加入 
// 队列满  重头删  后面加
// 入队
// 出队
// 限制大小
// 队列数据大小
// Demo实现
// Date数据段固定  
// 链表头  -    数据段   -    链表尾
//       出栈         数据插入
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#define QUEUE_DATA_MAX_SIZE 1024

typedef struct _queue_data {
    struct _queue_data *next;
    int size;
    char data[QUEUE_DATA_MAX_SIZE];
}queue_data;

typedef struct _queue {
    struct _queue_data *front;
    struct _queue_data *rear;
    int queue_len;
}queue;

extern int queue_type;
extern int queue_len;

queue * queue_init(int _type,int _queue_len);
void queue_destroy(); 
int queue_enqueue(queue *q, const char *data, int data_size);
int queue_pop(queue *q, char *data, int *data_size);