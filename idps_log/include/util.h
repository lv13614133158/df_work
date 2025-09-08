/*
 * @Descripttion: 
 * @version: V0.0
 * @Author: qihoo360
 * @Date: 1969-12-31 19:00:00
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2021-08-09 02:33:25
 */ 

#ifndef __UTILS_H__
#define __UTILS_H__
#include <stdlib.h>
#include <stdbool.h>

typedef struct list_elmt_ 
{
	void *data;	
	struct list_elmt_ *next;
}list_elmt;

typedef struct list_
{
	unsigned int size;
	int (*match)(const void *key1,const void *key2);
	void (*destroy)(void *data);
	list_elmt *head;
	list_elmt *tail;
}list;
 /**
 * @description: function type def 
 * @param {*}
 * @return {*}
 */
enum _websocketRpcType{
    websocketrpc_intptr,
    websocketrpc_charptr,
    websocketrpc_longptr,
    websocketrpc_longlongptr,
    websocketrpc_doubleptr,
    websocketrpc_shortptr,
    websocketrpc_voidptr,

    websocketrpc_int,
    websocketrpc_char,
    websocketrpc_long,
    websocketrpc_longlong,
    websocketrpc_double,
    websocketrpc_short,
    websocketrpc_void,
    websocketrpc_bool,

    websocketrpc_space1,
    websocketrpc_space2,
    websocketrpc_space3
};
/* public interface */
void list_init(list *_list,void (*destroy)(void *data));
void list_destroy(list *_list);
void destroy(list *_list);
int  list_ins_next(list *_list,list_elmt *element,const void *data);
list_elmt *list_rem_head(list* _list);
int  list_rem_next(list *_list,list_elmt *element,void **data);
void list_delindex(list *_list,list_elmt* old_elmt);
bool addliststring(list *_inputlist,char* _inputstring);
bool delliststring(list *_inputlist,char* _inputstring);
int list_search(list *_list,void* data);
int list_search_delete(list *_list,void*data);
int list_search_all(list * _list);
#define list_size(list)	((list)->size)
#define list_head(list) ((list)->head)
#define list_tail(list) ((list)->tail)
#define list_is_head(list,element)	((element)==(list)->head?1:0)
#define list_is_tail(list,element)	((element)==(list)->tail?1:0)
#define list_data(element)		((element)->data)
#define list_next(element)		((element)->next)
#endif

