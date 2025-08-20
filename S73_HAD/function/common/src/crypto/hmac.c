#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include "openssl/md5.h"
#include "openssl/sha.h"
#include "myHmac.h"


#define READ_DATA_SIZE	    1024
#define MD5_SIZE		    16
#define SHA1_SIZE		    20
#define KEY_IOPAD_SIZE      64
#define KEY_IOPAD_SIZE128   128

//Convert an array to a string
static void _array_to_string(const unsigned char *array,const int len,char *string)
{
    int i;
    for(i = 0; i < len; i++)
	{
		snprintf(string + i*2, 2+1, "%02x", array[i]);
	}
}

static int is_dir_exist(const char *dir_path)
{
    DIR *dirp;
    if(dir_path == NULL)
        return -1;
    dirp = opendir(dir_path);
    if(dirp == NULL)
    {
        return -1;
    }
	closedir(dirp);
    return 0;
}
/*
unsigned char*  text;                pointer to data stream
int             text_len;            length of data stream
unsigned char*  key;                 pointer to authentication key
int             key_len;             length of authentication key
unsigned char*  digest;              caller digest to be filled in
*/
void hmac_md5(unsigned char *key, int key_len,
    unsigned char *text, int text_len, unsigned char *hmac)
{
    MD5_CTX context;
    unsigned char k_ipad[KEY_IOPAD_SIZE];    /* inner padding - key XORd with ipad  */
    unsigned char k_opad[KEY_IOPAD_SIZE];    /* outer padding - key XORd with opad */
    int i;

    /*
    * the HMAC_MD5 transform looks like:
    *
    * MD5(K XOR opad, MD5(K XOR ipad, text))
    *
    * where K is an n byte key
    * ipad is the byte 0x36 repeated 64 times

    * opad is the byte 0x5c repeated 64 times
    * and text is the data being protected
    */

    /* start out by storing key in pads */
    memset( k_ipad, 0, sizeof(k_ipad));
    memset( k_opad, 0, sizeof(k_opad));
    memcpy( k_ipad, key, key_len);
    memcpy( k_opad, key, key_len);
    
    /* XOR key with ipad and opad values */
    for (i = 0; i < KEY_IOPAD_SIZE; i++) {
        k_ipad[i] ^= 0x36;
        k_opad[i] ^= 0x5c;
    }
    
    // perform inner MD5
    MD5_Init(&context);                    /* init context for 1st pass */
    MD5_Update(&context, k_ipad, KEY_IOPAD_SIZE);      /* start with inner pad */
    MD5_Update(&context, (unsigned char*)text, text_len); /* then text of datagram */
    MD5_Final(hmac, &context);             /* finish up 1st pass */
    
    // perform outer MD5
    MD5_Init(&context);                   /* init context for 2nd pass */
    MD5_Update(&context, k_opad, KEY_IOPAD_SIZE);     /* start with outer pad */
    MD5_Update(&context, hmac, MD5_DIGEST_SIZE);     /* then results of 1st hash */
    MD5_Final(hmac, &context);          /* finish up 2nd pass */
}

// MD5 消息摘要，不加key
/**
 * compute the value of a string
 * @param  dest_str
 * @param  dest_len
 * @param  md5_str
 */
int Compute_string_md5(const char *dest_str, unsigned int dest_len, char *md5_str)
{
	int i;
	unsigned char md5_value[MD5_SIZE];
	MD5_CTX md5;

	// init md5
	MD5_Init(&md5);

	MD5_Update(&md5, dest_str, dest_len);

	MD5_Final(md5_value,&md5);

	// convert md5 value to md5 string
	_array_to_string(md5_value, MD5_SIZE, md5_str);
	return 0;
}

/**
 * compute the value of a file
 * @param  file_path
 * @param  md5_str
 * @return 0: ok, -1: fail
 */
int Compute_file_md5(const char *file_path, char *md5_str)
{
	int fd;
	int ret;
	unsigned char data[READ_DATA_SIZE];
	unsigned char md5_value[MD5_SIZE];
	MD5_CTX md5;

    // 判度文件而非文件夹
    if(file_path == NULL)
        return -1;
    ret = is_dir_exist(file_path);
    if(ret == 0)
    {
        perror("Path can only be a file\n");
        return -3;
    }

	fd = open(file_path, O_RDONLY);
	if (-1 == fd)
	{
		perror("open");
		return -1;
	}

	// init md5
	MD5_Init(&md5);

	while (1)
	{
		ret = read(fd, data, READ_DATA_SIZE);
		if (-1 == ret)
		{
			perror("read");
			close(fd);
			return -1;
		}

		MD5_Update(&md5, data, ret);

		if (0 == ret || ret < READ_DATA_SIZE)
		{
			break;
		}
	}
	close(fd);

	MD5_Final(md5_value,&md5);
	// convert md5 value to md5 string
	_array_to_string(md5_value, MD5_SIZE, md5_str);
	return 0;
}

// get file fingerprint  user API,return 20Byte array to hash_out
int Compute_file_sha1(const char *filePath,unsigned char *hash_out)
{
    SHA_CTX c;
    int i,ret;
    unsigned char buf[READ_DATA_SIZE]={0};
    unsigned char sha1_value[SHA1_SIZE]={0};
    FILE *fp;

    if(filePath == NULL)
        return -1;
    ret = is_dir_exist(filePath);
    if(ret == 0)
    {
        perror("Path can only be a file\n");
        return -3;
    }
    fp = fopen(filePath,"r");
    if(fp == NULL)
    {
        perror("open\n");
        return -2;   
    }
   
    SHA1_Init(&c);

    while(!feof(fp))/*判断是否结束，这里会有个文件结束符*/
    {
        i = fread(buf,1,READ_DATA_SIZE,fp);
        if (i <= 0)
            break;
        SHA1_Update(&c, buf, (unsigned long)i);
    }  
    fclose(fp);

    SHA1_Final(sha1_value, &c);
    _array_to_string(sha1_value, SHA1_SIZE, hash_out);
    return 0;
}

int get_string_sha1(const char *dest_str, unsigned int dest_len, unsigned char *sha1_str)
{
    if(dest_str == NULL)
        return -1;

	unsigned char sha1_value[20];
	SHA_CTX sha1;
    SHA1_Init(&sha1);
    SHA1_Update(&sha1,(unsigned char *)dest_str, dest_len);
    SHA1_Final(sha1_value,&sha1);
    memcpy(sha1_str,sha1_value,20);

	return 0;
}

