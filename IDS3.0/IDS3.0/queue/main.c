#include "stdio.h"
#include "queue.h"
#include <string.h>
int main()
{
    queue *q;
    q=queue_init(1,10);
    char * buff="123456789";
    int len= strlen(buff);
    for(int i=0;i<15;i++)
    {
        queue_enqueue(q,buff,len);
        printf("queue_enqueue len= [%d]\n",q->queue_len);
    }
    char data[1024];

    int datalen;
    for(int i=0;i<15;i++)
    {
        queue_pop(q,data,&datalen);
        printf("queue_pop len= [%d]\n",q->queue_len);
    }

    return  0;
}