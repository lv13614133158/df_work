#include "queue.h"

// 初始化队列
int queue_type = 0;
int queue_len = 1000;
queue * queue_init(int _type,int _queue_len){
    queue *q = (queue *)malloc(sizeof(queue));
    q->front = NULL;
    q->rear = NULL;
    q->queue_len = 0;
    queue_type = _type;
    queue_len = _queue_len;
    return q;
}

// 销毁队列
void queue_destroy(queue *q) {
    while (q->front != NULL) {
        queue_data *temp = q->front;
        q->front = q->front->next; 
        free(temp);
    }
    q->rear = NULL;
    q->queue_len = 0;
}

int queue_enqueue(queue *q, const char *data, int data_size) {
    // 检查数据大小
    if (data_size > QUEUE_DATA_MAX_SIZE) {
        return -1; // 数据过大
    }
    if(q->queue_len >= queue_len && queue_type==1 )
    {
        return -2;//队列满
    }
    queue_data *new_node = (queue_data *)malloc(sizeof(queue_data));
    if (!new_node) {
        return -3; // 内存分配失败
    }

    new_node->next = NULL; 
    new_node->size = data_size;
    memcpy(new_node->data, data, data_size);
    
    if(q->queue_len>=queue_len && queue_type==0 )
    {
            queue_data *temp = q->front;
            q->front = q->front->next;
            free(temp);
    }
    // 从队尾插入
    if (q->front == NULL) {
        q->front = new_node;
        q->rear = new_node;
    } else {
        q->rear->next = new_node;
        q->rear = new_node;
    }
    q->queue_len++;
    return 0; 
}

int queue_pop(queue *q, char *data, int *data_size) {
    // 检查队列是否为空
    if (q->front == NULL) {
        return -1;
    }
    queue_data *temp = q->front;
    *data_size = temp->size;
    memcpy(data, temp->data, temp->size);
    data[temp->size] = '\0';
    // 更新队列头
    q->front = q->front->next;
    
    // 如果队列变空，更新队尾指针
    if (q->front == NULL) {
        q->rear = NULL;
    }
    
    free(temp);
    q->queue_len--;

    return 0; 
}