#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include "util/log_util.h"
#include "FileWrapper.h"
#include "spdloglib.h"
// Check for file existence
#define F_OK 0
/* Check for read permission */
#define R_OK 4
/**
 * 获取文件的大小
 */ 
int file_size(char* filename)
{
    struct stat statbuf;
    stat(filename,&statbuf);
    int size=statbuf.st_size;
    return size;
}
/**
 * 根据指定路径对文件进行处理
 * 注意返回空间的释放  NULL不需要释放
 */ 
FileWrapper_t *processFileWrapper(char* path){
    FileWrapper_t *FileWrapper = (FileWrapper_t *)malloc(sizeof(FileWrapper_t));
    int            pathlength = strlen(path);
    if(FileWrapper == NULL)
        return NULL;
    FileWrapper->errorMsg = (char*)malloc(pathlength + 32);
    memset(FileWrapper->errorMsg,0,pathlength + 32);
    if (pathlength <= 2) {
        memcpy(FileWrapper->errorMsg,"文件路径为空",strlen("文件路径为空"));
        FileWrapper->fileData = (char*)malloc(1);
        return FileWrapper;
    }
    if (access(path, F_OK) != 0) {
        memcpy(FileWrapper->errorMsg,"此路径文件不存在:",strlen("此路径文件不存在:"));
        strcat(FileWrapper->errorMsg,path);
        FileWrapper->fileData = (char*)malloc(1);
        return FileWrapper;
    }
    if (access(path, R_OK) != 0) {
        memcpy(FileWrapper->errorMsg,"此路径文件无读权限:",strlen("此路径文件无读权限:"));
        strcat(FileWrapper->errorMsg,path);
        FileWrapper->fileData = (char*)malloc(1);
        return FileWrapper;
    }
    FileWrapper->fileData = (char *)malloc(file_size(path) + 32);
    if(FileWrapper->fileData == NULL){
        log_i("networkmanager module","read file malloc fail");
    }
    memset(FileWrapper->fileData,0,file_size(path) + 32);
    FILE *readhandle = fopen(path,"rb");
    if(readhandle == NULL){
        memcpy(FileWrapper->errorMsg,"此路径文件不存在:",strlen("此路径文件不存在:"));
        return FileWrapper;
    }
    long long validCount = fread(FileWrapper->fileData,sizeof(char),file_size(path) + 32,readhandle);
    FileWrapper->fileDataLen = validCount;
    fclose(readhandle);
    return FileWrapper;
}