#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include "util.h"
#include <stdbool.h>


void destroy(list *_list)
{
	void *data;
	while(list_size(_list)>0)
	{
		if(list_rem_next(_list,NULL,(void **)&data)==0 && _list->destroy!=NULL)
		{
			_list->destroy(data);
		}	
	}
	memset(_list,0,sizeof(list));
	return;
}
void list_init(list *_list,void (*destroy)(void *data))
{
	_list->size=0;
	_list->destroy = destroy;
	_list->head=NULL;
	_list->tail=NULL;
	return;
}
/**
 * @name:   list_destroy
 * @Author: qihoo360
 * @msg:    other place wirte wrong,then rename destroy
 * @param  
 * @return: 
 */
void list_destroy(list *_list){
	destroy(_list);
}
int list_ins_next(list *_list,list_elmt *element,const void *data)
{
	list_elmt *new_element;
	
	if((new_element = (list_elmt *)malloc(sizeof(list_elmt))) == NULL)
		return -1;
	new_element->next = NULL;
	new_element->data = (void*)data;
	
	if(element==NULL)
	{
		if(list_size(_list)==0)
			_list->tail = new_element;
		new_element->next=_list->head;
		_list->head = new_element;
	}
	else
	{
		if(element->next == NULL)
		{
			_list->tail = new_element;			
		}
		new_element->next = element->next;
		element->next = new_element;
	}
	_list->size++;
	return 0;
}

list_elmt *list_rem_head(list* _list){

	if(list_size(_list) == 0){
		return NULL;
	}
    list_elmt *p = _list->head;
    _list->head = p->next;
    (_list->size)--;
    if (_list->size == 0)
    {
        _list->tail = NULL;
    }
    return p;
}

int list_rem_next(list *_list,list_elmt *element,void **data)
{
	list_elmt *old_element;
	if(list_size(_list)==0)
	{
		return -1;	
	}
	if(element == NULL)
	{
		*data = _list->head->data;
		old_element = _list->head;
		_list->head = _list->head->next;
		if(list_size(_list)==1)
		{
			_list->tail = NULL;
		}		
	}
	else
	{
		if(element->next == NULL)
		{
			return -1;				
		}
		*data = element->next->data;
		old_element = element->next;
		element->next =  element->next->next;
		if(element->next ==NULL)
		{
			_list->tail=element;
		}
	}
	free(old_element);
	_list->size--;
	return 0;
}
/**
 * @name:   search_elmt
 * @Author: qihoo360
 * @msg: 
 * @param  
 * @return: 
 */
static inline char* search_elmt(list *_list,char* _dnsstring){
	list_elmt *head = _list->head;
	if(head ==NULL)
		return NULL;
	list_elmt *cur = head;
	while(cur != NULL)
	{
		char *body =	(char*)cur->data;
		if(strcmp(body,_dnsstring) == 0){
			return body;
		}
		cur = cur->next;
	}
	return NULL;
}
/**
 * @name:   list_delindex
 * @Author: qihoo360
 * @msg: 
 * @param  
 * @return: 
 */
void list_delindex(list *_list,list_elmt* old_elmt)
{
	void *data;
	if(list_rem_next(_list,old_elmt,(void **)&data)==0 && _list->destroy!=NULL)
	{
		_list->destroy(data);
	}	
	return;
}
/**
 * @name:   addliststring
 * @Author: qihoo360
 * @msg: 
 * @param 
 * @return: 
 */
bool addliststring(list *_inputlist,char* _inputstring){
	if(NULL == search_elmt(_inputlist,_inputstring)){
		char* node = (char*)malloc(strlen(_inputstring) + 2);		
		memset(node,0,strlen(_inputstring)+2);
		memcpy(node,_inputstring,strlen(_inputstring));
		list_ins_next(_inputlist,NULL,node);
		return true;
	}
	return false;
}
/**
 * @name:   delliststring
 * @Author: qihoo360
 * @msg: 
 * @param  
 * @return: 
 */
bool delliststring(list *_inputlist,char* _inputstring){
	list_elmt *old_elmt = NULL;
	list_elmt *cur_elmt = list_head(_inputlist);
	while(cur_elmt != NULL){
		char *stringget = cur_elmt->data;
		if(strcmp(stringget,_inputstring) == 0)//equal
		{
			list_delindex(_inputlist,old_elmt);
			return true;	
		}
		old_elmt = cur_elmt;
		cur_elmt = cur_elmt->next;
	}
	return false;

}
int list_search(list *_list,void* data)
{
	if(list_size(_list)==0)
	{
		return 0;	
	}
	list_elmt * list_temp;
	list_temp = _list->head;
	while(list_temp!= NULL)
	{
		if(strcmp((const char*)list_temp->data,(const char*)data) == 0)
		{
			return 1;
		}
		list_temp = list_temp->next;
	}
	return 0;
}
int list_search_delete(list *_list,void*data)
{
	if(list_size(_list)==0)
	{
		return 0;	
	}
	list_elmt * list_temp;
	list_elmt * list_temp_last;
	list_temp_last = _list->head;
	list_temp = list_next(_list->head);
	if( strcmp((const char*)list_data(_list->head),(const char*)data) == 0 )
	{
		void *data = NULL;
		list_rem_next(_list, NULL, (void**)&data);
		data = NULL;
		return 1;
	}
	while(list_temp!= NULL)
	{
		if(strcmp((const char*)list_temp->data,(const char*)data) == 0)
		{
			void *data = NULL;
			list_rem_next(_list, list_temp_last, (void**)&data);
			data = NULL;
			return 1;
		}
		list_temp_last = list_temp;
		list_temp = list_temp->next;
	}
	return 0;
}

int list_search_all(list * _list)
{
	if(list_size(_list)==0)
	{
		return 0;	
	}
	list_elmt * list_temp;
	list_temp = list_next(_list->head);
	while(list_temp!= NULL)
	{
		list_temp = list_temp->next;
	}
	return 1;
}








