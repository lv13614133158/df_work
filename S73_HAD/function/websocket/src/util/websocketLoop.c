#include <pthread.h>
#include "websocketLoop.h"
#include <stdio.h>
#include <string.h>

static pthread_mutex_t    map_operate_lock = PTHREAD_MUTEX_INITIALIZER;

typedef struct _map 
{
    int size;
    map_elmt array[WBSMAP_MAX];	
}map;
static map map_data = {0};

// 找空位 位置pos
static int _findEmptyPos()
{
    if(map_data.size >= WBSMAP_MAX)
    {
        printf("Element count already max value!\n");
        return -2;
    }

    for(int i=0; i< WBSMAP_MAX; i++)
    {
        if(!map_data.array[i].used)
        {
           return i;
        }
    }
    return -1;
}

// key找 位置pos
static int _findMapPos(long long key)
{
    for(int i=0, j=1; i< WBSMAP_MAX; i++)
    {
        if(map_data.array[i].used)
        {
            if(map_data.array[i].key == key)
            {
                return i;
            }
            if((++j) > map_data.size)
            {
                return -1;
            }
        }
    }
    return -1;
}

// 位置pos 找value
static char* _findMapElmt(int index)
{
    if(index >= WBSMAP_MAX || index < 0)
        return NULL;

    if(map_data.array[index].used && map_data.array[index].data)
        return map_data.array[index].data;
    
    return NULL;
}

// key 找value
static char* _findMapElmtEx(long long key)
{
    return _findMapElmt(_findMapPos(key));
}

// 位置pos 添加key value
static int _addMapElmt(int index,long long key, char *data, bool send_only_once)
{
    int data_len = 0;

    if(index >= WBSMAP_MAX || map_data.array[index].used)
        return -1;

    data_len = strlen(data);

    map_data.array[index].data = data;
    map_data.array[index].key = key;
    map_data.array[index].used  = 1;
    map_data.array[index].len = data_len;
    map_data.array[index].send_only_once = send_only_once;
    map_data.size++;

    return map_data.size;
}

// 位置pos 删除
static int _delMapElmt(int index)
{
    if(index >= WBSMAP_MAX)
        return -1;

    if(map_data.array[index].used)
    {
        if(map_data.array[index].data)
        {
            free(map_data.array[index].data);
            memset(&map_data.array[index], 0, sizeof(map_elmt));
            map_data.size--;
            return map_data.size;
        }
    }

    return -1;
}

int wbsGetSize()
{
    return map_data.size;
}

int wbsAddMap(long long key, char *data, bool send_only_once)
{
    int index =-1;
    int empty_pos = 0;

    pthread_mutex_lock(&map_operate_lock);
    empty_pos = _findEmptyPos();
    if (empty_pos < 0)
    {
        /*clear map*/
        for(int i=0; i< WBSMAP_MAX; i++)
        {
            if(map_data.array[i].used)
            {
                _delMapElmt(i);
            }
        }
        map_data.size = 0;
        empty_pos = _findEmptyPos();
        printf("map overflow! clear!\n");
    }
    index = _findMapPos(key);
    if(index < 0)
    {
        if(_addMapElmt(empty_pos, key, data, send_only_once) < 0)
        {
            printf("Element add failed!\n");
        }
        else
        {
            pthread_mutex_unlock(&map_operate_lock);
            return 0;
        }
    }
    else
    {
        printf("Elment already exists!\n");
    }
    pthread_mutex_unlock(&map_operate_lock);
    return -1;
}

int wbsDelMap(long long key)
{
    int index =-1;
    pthread_mutex_lock(&map_operate_lock);
    index = _findMapPos(key);
    if(index >= 0)
    {
        if(_delMapElmt(index) < 0)
        {
            printf("Element delect failed!\n");
        }
        else
        {
            pthread_mutex_unlock(&map_operate_lock);
            return 0;
        } 
    }
    else
    {
        printf("Elment not exists!\n");
    }
    pthread_mutex_unlock(&map_operate_lock);
    return -1;
}

char* wbsGetMapEx(long long key)
{
    char *data =NULL;
    pthread_mutex_lock(&map_operate_lock);
    char *elmt = _findMapElmtEx(key);
    if(elmt)
    {
        int len = strlen(elmt);
        data = (char*)malloc(len);
        memcpy(data, elmt, len);
    }
    pthread_mutex_unlock(&map_operate_lock);
    return data;
}

int wbsGetMap(int index, map_elmt *map_buff)
{
    char *data = NULL;
    int ret = -1;

    if(index >= WBSMAP_MAX || index < 0 || map_buff == NULL)
        return ret;

    pthread_mutex_lock(&map_operate_lock);
    if (map_data.array[index].used)
    {
        if (map_data.array[index].ready_retry_send == 0)
        {
            map_data.array[index].ready_retry_send = 1;

            *map_buff = map_data.array[index];

            data = (char*)malloc(map_data.array[index].len);
            if (data)
            {
                memcpy(data, map_data.array[index].data, map_data.array[index].len);
                map_buff->data = data;
                ret = index;
            }
        }
    }
    pthread_mutex_unlock(&map_operate_lock);

    return ret;
}

int wbsSetRetrySend(void)
{
    int i = 0;
    pthread_mutex_lock(&map_operate_lock);
    for(i = 0; i < WBSMAP_MAX; i++)
    {
        if (map_data.array[i].used && map_data.array[i].ready_retry_send)
        {
            map_data.array[i].ready_retry_send = 0;
        }
    }
    pthread_mutex_unlock(&map_operate_lock);
}

void wbsGetList(list *listTx)
{
    char *ins_data = NULL;

    pthread_mutex_lock(&map_operate_lock);
    for(int i=0, j=1; i< WBSMAP_MAX; i++)
    {
        if(map_data.array[i].used && j++)
        {
            if (map_data.array[i].data)
            {
                ins_data = (char *)malloc(map_data.array[i].len + 1);
                if (ins_data)
                {
                    memcpy(ins_data, map_data.array[i].data, map_data.array[i].len);
                    list_ins_next(listTx, NULL, ins_data);
                }
            }
            if(j > map_data.size)break;
        }
    }
    pthread_mutex_unlock(&map_operate_lock);
}

void wbsClearMap()
{
    pthread_mutex_lock(&map_operate_lock);
    for(int i=0, j=1; i< WBSMAP_MAX; i++)
    {
        if(map_data.array[i].used)
        {
            _delMapElmt(i);
        }
    }
    map_data.size = 0;
    pthread_mutex_unlock(&map_operate_lock);
}

void wbsPrintMap()
{
    printf("size:%d\n\n",map_data.size);
    for(int i=0, j=1; i< WBSMAP_MAX; i++)
    {
        if(map_data.array[i].used)
        {
            printf("key:%.3d,used:%d i:%lld len:%d\n",
                i, map_data.array[i].used, map_data.array[i].key, map_data.array[i].len);
            if(map_data.array[i].data)
                printf("data:%s",(char *)(map_data.array[i].data)); 
        } 
        if((++j) > map_data.size)return;
    }
}

int wbsInitMap(int size)
{
    return 0;
}

int wbsResizeMap(int size)
{
    return 0;
}
