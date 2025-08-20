#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/aes.h>
#include "base64.h"
#include "openAes.h"

#define S_AES_KEY  "0123456789abcdef"
#define S_AES_VI   "abcdef0123456789"
char *  s_aes_key = NULL;
char *  s_aes_vi = NULL;

static void printHex(unsigned char* out, int len);

void aes_set_key_iv(char *key, char *iv)
{
    if (s_aes_key == NULL)
    {
        memcpy(key, S_AES_KEY, 16);
    }else
    {
        memcpy(key, s_aes_key, 16);
    }


    if (s_aes_vi == NULL)
    {
        memcpy(iv, S_AES_VI, 16);
    }else
    {
        memcpy(iv, s_aes_vi, 16);
    }
}

static void aes_cbc128_encrypt(const unsigned char *in, size_t len, unsigned char *out)
{
    AES_KEY aes_key;
    unsigned char key[16];
    unsigned char iv[16];

    aes_set_key_iv(key,iv);
 
    AES_set_encrypt_key(key, 128, &aes_key);

    AES_cbc_encrypt(in, out, len, &aes_key, iv, AES_ENCRYPT);
}

static void aes_cbc128_decrypt(const unsigned char *in, size_t len, unsigned char *out)
{
    AES_KEY aes_key;
    unsigned char key[16];
    unsigned char iv[16];

    aes_set_key_iv(key,iv);
 
    AES_set_decrypt_key(key, 128, &aes_key);

    AES_cbc_encrypt(in, out, len, &aes_key, iv, AES_DECRYPT);
}

static void printHex(unsigned char* out, int len)
{
    for (int i = 0; i < len; i++) {
        printf("%.02X", out[i]);
    }
    printf("\n");
}

// 先AES cbc128加密再base64 
char* aes_char_write(char* input)
{
    size_t in_len = 0, out_len = 0;
    char* str_base64 = NULL;
    if( input )
    {
        in_len  = strlen(input);
        out_len = (((in_len)>>4)<<4)+((in_len%16)?16:0);
        unsigned char* str_aes = (unsigned char*)malloc(out_len);
        if(str_aes)
        {
            memset(str_aes, 0, out_len);
            aes_cbc128_encrypt(input, in_len, str_aes);
            //c = strlen(str_aes);// AES加密后容易出现0但是长度是固定的
           // printHex(str_aes, out_len);
            in_len = out_len;
            str_base64 = base64_encode((const unsigned char *)str_aes, in_len, &out_len);
            free(str_aes);
        }
    }
    return str_base64;
}

// 先反base64 再AES cbc128解密
char* aes_char_read(char* input, int max_len)
{
    size_t in_len = 0, out_len = 0;
    char* output = NULL, *str_base64 = NULL;
    if(input)
    {
        in_len = strlen(input);
        str_base64 = base64_decode((const unsigned char *)input, in_len, &out_len);
        //printHex(str_base64, out_len);
        if(str_base64 && out_len>=0)
        {
            output =  (unsigned char*)malloc(out_len);
            if(output)
            {
                aes_cbc128_decrypt(str_base64, out_len, output);
            }
            free(str_base64);
        }
    }

    return output;
}



int aes_init(char*key, char *iv)
{
    if(key == NULL || iv == NULL)
    {
        return 1;
    }

    char * str_aes_key = NULL;
    char * str_aes_vi = NULL;
    int key_len = 0;
    int iv_len = 0;
    
    key_len = strlen(key);
    iv_len = strlen(iv);
    if(key_len != 16 || iv_len != 16)
    {
        return 2;
    }

    str_aes_key =  (unsigned char*)malloc(key_len+1);
    if(str_aes_key == NULL)
    {
        return 3;
    }
 
    str_aes_vi =  (unsigned char*)malloc(iv_len+1);
    if(str_aes_vi == NULL)
    {
        free(str_aes_key);
        return 3;
    }

    memcpy(str_aes_key, key,key_len+1);
    memcpy(str_aes_vi, iv,iv_len+1);
    s_aes_key = str_aes_key;
    s_aes_vi = str_aes_vi;

    return 0;
}
void aes_delete()
{
    if(s_aes_key != NULL)
    {
        free(s_aes_key);
        s_aes_key = NULL;
    }

    if(s_aes_vi != NULL)
    {
        free(s_aes_vi);
        s_aes_vi = NULL;
    }
  
}

