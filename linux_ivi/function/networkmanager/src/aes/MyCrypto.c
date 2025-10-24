#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include "MyCrypto.h"
#include "openssl/md5.h"
#include <stdbool.h>
#include "spdloglib.h"
#include "myHmac.h"

#define MD5_SIZE 16

static uint8_t SET_WORKDIR_FLAG=0;
static char WORKDIR[256]={0};
static char fileName[255]="keystore";

static bool findIndexExist(char *index);
static bool getStructFromFile(char *index,cryptoNode_t *node);
static bool insertDataToFile(cryptoNode_t *pdata);
 /*
 * function: setWorkDirectory
 * input   :
 *          @dir:存在的工作目录的绝对路径
 *          
 * output  :成功返回0，失败返回负数 
 * decla   :设置加密模块工作目录，并在该目录下创建keystore文件，用于密钥存储记录
 */
int setWorkDirectory(const char *dir)
{
	if(dir == NULL)
		return -1;
    char temp[255];
    DIR *pdir;
    //判断dir是否存在，如果存在是否是目录
    if((pdir = opendir(dir)) == NULL)
    {
        return -2;//目录不存在
    }
    closedir(pdir);

    strncpy(temp,dir,sizeof(temp)-1);
	if(temp[strlen(temp)-1]!='/')
		temp[strlen(temp)]='/';
	SET_WORKDIR_FLAG=1;
    snprintf(WORKDIR,sizeof(WORKDIR),"%s%s",temp,fileName);
	// printf("path:%s\n",WORKDIR);

    //file not exist
    if(access(WORKDIR,F_OK) != 0)
    {
        FILE *fp;
        fp = fopen(WORKDIR,"w+");
        if(fp == NULL)
        {
            return -1;
        }
        fclose(fp);
        char spdlog[256] = {0};
        snprintf(spdlog,256,"create file success\n");
		log_i("CryptogramModule", spdlog);
    }
    else{
        char spdlog[256] = {0};
        snprintf(spdlog,256,"file is exist\n");
		log_i("CryptogramModule", spdlog);
    }
    return 0;
}

/*
*   function: storeKey
*   存储key&iv字符串，返回索引，便于下次加密解密使用
*   mode：存储的模式选择
*       1：存储到文件
*       2：存储到白盒(暂未实现)
*       3：存储到se
*/

 /*
 * function: storeKey
 * input   :
 *          @mode: 存储模式的选择
 *              1：存储到文件  2存储到白盒(暂未实现) 3存储到SE(暂未实现)
 *          @data：待存储字符串
 *          @outbuf：接收返回字符串
 *          
 * output  :成功返回true，失败返回false
 * decla   :存储key&iv字符串，便于加密解密
 */
bool storeKey(int mode,const char *data,char *outbuf)
{
    FILE *fp;
    char summary[33];
    bool ret;
    cryptoNode_t dataNode;
    if(data == NULL)
    {
        return false;
    }
    Compute_string_md5(data,strlen(data),summary);
    memset(&dataNode,0,sizeof(cryptoNode_t));
    ret = findIndexExist(summary);
    //索引已经存在
    if(ret == true)
    {
        char spdlog[256] = {0};
        snprintf(spdlog,256,"key has exist\n");
		log_i("CryptogramModule", spdlog);
        memcpy(outbuf,summary,strlen(summary));
        return true;
    }
    //存文件
    if(mode == 1)
    {       
        dataNode.mode = 1;
        dataNode.token = 0; //存文件时文件的token值没有用，默认赋值为0.
        dataNode.flag = true;
        strncpy(dataNode.index,summary,strlen(summary));
        strncpy(dataNode.data,data,strlen(data));
        char spdlog[256] = {0};
        snprintf(spdlog,256,"mode:%d flag:%d token:%d  index:%s  key&iv:%s   data:%s\n ",dataNode.mode,dataNode.flag,dataNode.token,dataNode.index,dataNode.data,data);
		log_i("CryptogramModule", spdlog);
    }
    else if(mode == 3) //存se
    {
        char spdlog[256] = {0};
        snprintf(spdlog,256,"调用存储密钥到se的API接口\n");
		log_i("CryptogramModule", spdlog);
        // int token;
        // dataNode.mode = 3;
        //dataNode.flag = true;
        // char key[17]={0},iv[17]={0};
        // memcpy(key,data,16);
        // memcpy(dataNode.data,data+16,16); //存储iv向量
        // token = build_symmetrickey(TYPE_AES,LENGTH_AES_128,key);
        // dataNode.token = token;
        // strncpy(dataNode.index,summary,strlen(summary));
        
    }
    else
    {
        return false;
    }
    insertDataToFile(&dataNode);
    memcpy(outbuf,summary,strlen(summary));
    return true;   
}

 /*
 * function: encryption
 * input   :
 *          @data_in:待加密数据
 *          @index：索引字符串
 * output  :
 * decla   : 通过index对应的key对数据进行加密，失败返回NULL，
 *          非NULL情况要对返回指针进行free释放，防止内存泄露
 */
cypher_t *encryption(cypher_t *data_in,char *index)
{
    cryptoNode_t eNode;
    cypher_t *edata=NULL;
    bool ret;
    ret = getStructFromFile(index,&eNode);
    if(ret == false)
    {
        char spdlog[256] = {0};
        snprintf(spdlog,256,"not foud key&Iv\n");
		log_i("CryptogramModule", spdlog);
        return NULL;
    }
    if(eNode.mode == 1)
    {
        char key[17]={0};
        char iv[17]={0};
        strncpy(key,eNode.data,16);
        strncpy(iv,eNode.data+16,16);
        edata = aes_cbc_encrypt(data_in,key,iv);
    }
    else if(eNode.mode == 3)
    {
        // int token;
        // char iv[17]={0};
        // char out[1024];
        // int outlength;
        // token = eNode.token;
        // memcpy(iv,eNode.data,16);
        // // 需要检查填充方式是否一致，超过65535字节是否有异常
        // aes_enc_cipher(TYPE_AES,token,data_in->data,data_in->len_data, out,&outlength,iv,16);
        // edata = malloc(sizeof(int)+outlength);
        // if(edata == NULL)
        // {
        //     return NULL;
        // }
        // edata->len_data = outlength;
        // memcpy(edata->data,out,edata->len_data);
    }
    return edata; 
}

 /*
 * function: decryption
 * input   :
 *          @data_in:待解密数据
 *          @index：索引字符串
 * output  : 
 * decla   : 通过index对应的key对密文进行解密，失败返回NULL，
 *          非NULL情况要对返回指针进行free释放，防止内存泄露
 */
cypher_t *decryption(cypher_t *data_in,char *index)
{
    cryptoNode_t eNode;
    cypher_t *ddata=NULL;
    bool ret;
    ret = getStructFromFile(index,&eNode);
    if(ret == false)
    {
        char spdlog[256] = {0};
        snprintf(spdlog,256,"not foud key&Iv\n");
		log_i("CryptogramModule", spdlog);
        return NULL;
    }
    if(eNode.mode == 1)
    {
        char key[17]={0};
        char iv[17]={0};
        strncpy(key,eNode.data,16);
        strncpy(iv,eNode.data+16,16);
        ddata = aes_cbc_decrypt(data_in,key,iv);

    }
    else if(eNode.mode == 3)
    {
        // int token;
        // char iv[17]={0};
        // char out[1024];
        // int outlength;
        // token = dNode.token;
        // memcpy(iv,dNode.data,16);
        // // 需要检查填充方式是否一致，超过65535字节是否有异常
        // aes_dec_cipher(TYPE_AES,token,data_in->data,data_in->len_data, out,&outlength,iv,16);
        // edata = malloc(sizeof(int)+outlength);
        // if(ddata == NULL)
        // {
        //     return NULL;
        // }
        // ddata->len_data = outlength;
        // memcpy(ddata->data,out,ddata->len_data);
    }
    return ddata;
}


 /*
 * function: deleteKey
 * input   :index:索引字符串，
 * output  :bool:如果成功返回true，失败返回false 
 * decla   : 通过索引删除存储的key，
 */
bool deleteKey(char *index)
{
    FILE *fp;
    cryptoNode_t cryptoNode;
    struct stat buf;
    int count,i,ret;
    stat(WORKDIR,&buf);
    count = buf.st_size/sizeof(cryptoNode_t);
    fp = fopen(WORKDIR,"rb+");
    for(i=0;i<count;i++)
    {
        memset(&cryptoNode,0,sizeof(cryptoNode_t));
        size_t result = fread(&cryptoNode,sizeof(cryptoNode_t),1,fp);
        ret = strncmp(cryptoNode.index,index,strlen(index));
        if(ret == 0 && cryptoNode.flag == true)
        {

            cryptoNode.flag = false;
            fseek(fp,i*sizeof(cryptoNode_t),SEEK_SET);
            fwrite(&cryptoNode,sizeof(cryptoNode_t),1,fp);
            fclose(fp);
            return true;
        }
    }
    fclose(fp);
    return false;
}

 /*
 * function: findIndexExist
 * input   :index:索引字符串，
 * output  :bool： 
 * decla   :仅仅判断索引index在文件中是否存在
 */
static bool findIndexExist(char *index)
{
    FILE *fp;
    cryptoNode_t cryptoNode;
    struct stat buf;
    int count,i,ret;
    stat(WORKDIR,&buf);
    count = buf.st_size/sizeof(cryptoNode_t);
    fp = fopen(WORKDIR,"rb+");
    for(i=0;i<count;i++)
    {
        memset(&cryptoNode,0,sizeof(cryptoNode_t));
        size_t result = fread(&cryptoNode,sizeof(cryptoNode_t),1,fp);
        ret = strncmp(cryptoNode.index,index,strlen(index));
        if(ret == 0 && cryptoNode.flag == true)
        {
            return true;
        }
    }
    fclose(fp);   
    return false;
}


 /*
 * function: getStructFromFile
 * input   :index:索引字符串，
 *          cryptoNode_t：结构体指针，接收返回的数据
 * output  :bool： 
 * decla   :通过索引index从文件中获取对应的结构体数据到node中
 */
static bool getStructFromFile(char *index,cryptoNode_t *node)
{
    FILE *fp;
    cryptoNode_t cryptoNode;
    struct stat buf;
    int count,i,ret;
    stat(WORKDIR,&buf);
    count = buf.st_size/sizeof(cryptoNode_t);
    fp = fopen(WORKDIR,"rb+");
    for(i=0;i<count;i++)
    {
        memset(&cryptoNode,0,sizeof(cryptoNode_t));
        size_t result = fread(&cryptoNode,sizeof(cryptoNode_t),1,fp);
        ret = strncmp(cryptoNode.index,index,strlen(index));
        if(ret == 0 && cryptoNode.flag == true)
        {
            memcpy(node,&cryptoNode,sizeof(cryptoNode_t));
            fclose(fp);
            return true;
        }
    }
    fclose(fp);
    return false;
}

 /*
 * function: insertDataToFile
 * input   :
 *          @cryptoNode_t：结构体指针，待插入文件的数据结构
 * output  :bool：成功返回true，失败返回false  
 * decla   :将pdata结构体插入到文件中，如果有flag==false的数据块则覆盖，没有则插入到文件最后
 */
static bool insertDataToFile(cryptoNode_t *pdata)
{
    FILE *fp = NULL;
    cryptoNode_t cryptoNode;
    struct stat buf;
    int count,i,ret;
    stat(WORKDIR,&buf);
    count = buf.st_size/sizeof(cryptoNode_t);
    fp = fopen(WORKDIR,"rb+");
    if(fp == NULL)
    {
        return false;
    }
    for(i = 0;i < count ;i++)
    {
        memset(&cryptoNode,0,sizeof(cryptoNode_t));
        size_t result = fread(&cryptoNode,sizeof(cryptoNode_t),1,fp);
        if(cryptoNode.flag == false) //key已经失效
        {
            fseek(fp,i*sizeof(cryptoNode_t),SEEK_SET);
            ret = fwrite(pdata,sizeof(cryptoNode_t),1,fp);
            fclose(fp);
            return true;
        }
    }
    fseek(fp,0,SEEK_END);
    ret = fwrite(pdata,sizeof(cryptoNode_t),1,fp);
    fclose(fp);
    return true;
}


//遍历结构体,测试方法，后期删除
int readFile()
{
    FILE *fp;
    cryptoNode_t cryptoNode;
    struct stat buf;
    int count,i;
    stat(WORKDIR,&buf);
    count = buf.st_size/sizeof(cryptoNode_t);
    fp = fopen(WORKDIR,"rb+");
    for(i=0;i<count;i++)
    {
        size_t result = fread(&cryptoNode,sizeof(cryptoNode_t),1,fp);
        char spdlog[256] = {0};
        snprintf(spdlog,256,"mode :%d  flag :%d token:%d  index:%s  data:%s\n",cryptoNode.mode, cryptoNode.flag,cryptoNode.token,cryptoNode.index,cryptoNode.data);
		log_i("CryptogramModule", spdlog);
        memset(&cryptoNode,0,sizeof(cryptoNode_t));
    }
    fclose(fp);   
    return 0;
}
