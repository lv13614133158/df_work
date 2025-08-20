//
// Created by tangxx on 9/25/19.
//
#include <stdio.h>
#include <errno.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "myHmac.h"
#include "cryptogram.h"
#include "spdloglib.h"

#define MD5_SIZE 16
static uint8_t SET_WORKDIR_FLAG=0;
static char WORKDIR[256]={0};

#define AES_FILE_NAME_INDEX  "d2e64f06c8855b171ed9c0d952b1"
#define AES_FILE_NAME_SN     "d2e64f06c8855b171ed9c0d952b2"
#define AES_FILE_NAME_MODE   "d2e64f06c8855b171ed9c0d952b3"

void writeKey(const char* _input, char* fileName){
	FILE *fp;
	char path[256]={0};
	
	memcpy(path, WORKDIR, strlen(WORKDIR));
	strncat(path,fileName,256-strlen(WORKDIR));
	fp = fopen(path,"wb+");
	if(fp)
	{
		int ret = fwrite(_input,1,strlen(_input),fp);
		if(ret < 0){
			log_e("networkmanager", "setIntManager IndexKey from db error -1");
		}
		fclose(fp);
	}
}

void readKey(char* _input, char* fileName){
	FILE *fp;
	char path[256]={0};//清零防止缓存

	memcpy(path, WORKDIR, strlen(WORKDIR));
	strncat(path,fileName,256-strlen(WORKDIR));
	fp = fopen(path,"rb+");
	if(fp)
	{
		int ret = fread(_input,1,32,fp);
		if(ret < 0){
			log_e("networkmanager", "getIntManager IndexKey from db error -1");
		}
		fclose(fp);
	}
}

void getIndexKeyNetWork(char* _input){
	readKey(_input, AES_FILE_NAME_INDEX);
}

void getIndexSN(char* _input){
	readKey(_input, AES_FILE_NAME_SN);
}

int  getMode(void){
	FILE *fp;
	char path[256]={0};//清零防止缓存

	memcpy(path, WORKDIR, strlen(WORKDIR));
	strncat(path,AES_FILE_NAME_MODE,256-strlen(WORKDIR));
	fp = fopen(path,"rb+");
	if(fp)
	{
		char _input[2]={0};
		int ret = fread(_input,1,1,fp);
		if(ret == 1){
			return _input[0];
		}
		else{
			log_e("networkmanager", "getIntManager IndexKey from db error -1");
		}
		fclose(fp);
	}
	return -1;
}

void setIndexKey(const char *indexKey){
	writeKey(indexKey, AES_FILE_NAME_INDEX);
}

void setIndexSN(const char *indexSN){
	writeKey(indexSN, AES_FILE_NAME_SN);
}

void setMode(int mode){
	char _input[2] = {0};
	_input[0] = mode;
	writeKey(_input, AES_FILE_NAME_MODE);
}

/*
*	@brief Storage key and IV vector
*	@param <mode>,Key storage method, 1 :Store to file, 2:Store to white box... 
			<data> Data to be stored (if IV not exist,the data length is 16byte,else the data length is 32byte string
	@return success return keyIndex,failure return NULL			
*/
char  *set_value(int mode, const char *data)
{
	int ret;
	static char summary[33]={0};

	if( data == NULL)
		return NULL;
	if(SET_WORKDIR_FLAG==0)
		return NULL;
	if(mode == 1) //Store data in a file
	{
		char path[256]={0};
        
		FILE *fp;
		
        Compute_string_md5(data,strlen(data),summary);
		memcpy(path,WORKDIR,strlen(WORKDIR));
		strncat(path,summary,256-strlen(WORKDIR));
		fp = fopen(path,"wb+");
		if(fp == NULL)
		{
			return NULL;
		}
		ret = fwrite(data,1,strlen(data),fp);
		if(ret < 0)
		{
			fclose(fp);
			return NULL;
		}
		fclose(fp);
		ret = 0;
		return summary;
	}
	else if(mode==2)//store data in write box
	{
		/* code */
	}
	return NULL;
}

/*
*	@brief get key(key & iv) string
*	@param <mode>,Key get method, 1 :fromto file, 2:from white box... 
			<keyIndex> Pointer returned by function X
			<outbuf> Receive key string
	@return success return 0,failure return negative
*
*
*/
int get_value(int mode ,const char *keyIndex,char *outbuf)
{
	int ret;
	if(keyIndex == NULL || outbuf == NULL|| SET_WORKDIR_FLAG==0)
	{
		return -1;
	}
	
	if(mode == 1) //get key_iv from file 
	{
		FILE *fp;
		char path[256]={0};//清零防止缓存
		memcpy(path,WORKDIR,strlen(WORKDIR));
		strncat(path,keyIndex,256-strlen(WORKDIR));
		fp = fopen(path,"rb+");
		if(fp == NULL)
		{
			return -1;
		}
		ret = fread(outbuf,1,32,fp);
		if(ret < 0)
		{
			return -2;
		}
		ret =0;
		fclose(fp);
	}
	else if(mode == 2)//from write box get key_iv
	{
		/* code */
	}
	return ret;
}



int delete_value(const char *data)
{
	char cmd[272]={0};

	snprintf(cmd, sizeof(cmd), "rm %s%s", WORKDIR, data);
	int result = system(cmd);

	return 0;
}

/*
*	@brief set work dir,  
*	
 */
int set_work_directory(const char *dir)
{
	if(dir == NULL)
		return -1;

	//dir not exist
	if (access(dir, F_OK) != 0)
	{
		mkdir(dir, 0777);
	}

	strncpy(WORKDIR,dir,sizeof(WORKDIR) - 1);
	if(WORKDIR[strlen(dir)-1]!='/')
		WORKDIR[strlen(dir)]='/';
	SET_WORKDIR_FLAG=1;
	return 0;
}

