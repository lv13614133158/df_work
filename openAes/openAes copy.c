#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/aes.h>
#include "base64.h"
#include "openAes.h"

#define S_AES_KEY  "0123456789abcdef"
#define S_AES_VI   "abcdef0123456789"

#define COMM_FILE_P  "/app/idps/conf/config/pfile"
#define COMM_FILE_P_SIZE 10240
#define COMM_FILE_K  "/app/idps/conf/config/kfile"
#define COMM_FILE_K_SIZE 5120
#define COMM_FILE_C  "/app/idps/conf/config/cfile"
#define COMM_FILE_C_SIZE 10240

static void printHex(unsigned char* out, int len);

static void aes_cbc128_encrypt(const unsigned char *in, size_t len, unsigned char *out)
{
    const unsigned char key[] = S_AES_KEY;
    unsigned char       iv[]  = S_AES_VI;
    AES_KEY aes_key;
    AES_set_encrypt_key(key, 128, &aes_key);

    AES_cbc_encrypt(in, out, len, &aes_key, iv, AES_ENCRYPT);
}

static void aes_cbc128_decrypt(const unsigned char *in, size_t len, unsigned char *out)
{
    const unsigned char key[] = S_AES_KEY;
    unsigned char       iv[]  = S_AES_VI;
    AES_KEY aes_key;
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
char* aes_config_write(char* input)
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
            printHex(str_aes, out_len);
            in_len = out_len;
            str_base64 = base64_encode((const unsigned char *)str_aes, in_len, &out_len);
            free(str_aes);
        }
    }
    return str_base64;
}

// 先反base64 再AES cbc128解密
char* aes_config_read(char* output, char* input, int max_len)
{
    size_t in_len = 0, out_len = 0;
    char* str_aes = NULL, *str_base64 = NULL;
    if(input && output)
    {
        in_len = strlen(input);
        str_base64 = base64_decode((const unsigned char *)input, in_len, &out_len);
        //printHex(str_base64, out_len);
        if(str_base64 && out_len>=0)
        {
            str_aes =  (unsigned char*)malloc(out_len);
            if(str_aes)
            {
                aes_cbc128_decrypt(str_base64, out_len, str_aes);
                strncpy(output, str_aes, max_len);
                //printf("%s\n", output);
                free(str_aes);
            }
            free(str_base64);
        }
    }

    return output;
}

char* aes_read_sslcert(char* output)
{
    FILE *file = fopen(COMM_FILE_P, "r");
    if (file == NULL) {
        perror("Failed to open file");
        return NULL;
    }

    // 获取文件大小
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    // 分配内存以保存文件内容
    char *buffer = (char*) malloc(fileSize + 1);
    if (buffer == NULL) {
        perror("Failed to allocate memory");
        fclose(file);
        return NULL;
    }

    // 读取文件内容到内存
    size_t bytesRead = fread(buffer, 1, fileSize, file);
    if (bytesRead != fileSize) {
        perror("Failed to read file");
        free(buffer);
        fclose(file);
        return NULL;
    }

    // 添加空字符以结束字符串
    buffer[bytesRead] = '\0';

    // 此时，buffer中包含了文件的全部内容
    //printf("==> pfile File content:\n%s, size: %d\n", buffer, strlen(buffer));
    //AES解密
    aes_config_read(output, buffer, COMM_FILE_P_SIZE);  

     // 清理
    free(buffer);
    fclose(file);
    return output;
}

char* aes_read_sslkey(char* output)
{
    FILE *file = fopen(COMM_FILE_K, "r");
    if (file == NULL) {
        perror("Failed to open file");
        return NULL;
    }

    // 获取文件大小
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    // 分配内存以保存文件内容
    char *buffer = (char*) malloc(fileSize + 1);
    if (buffer == NULL) {
        perror("Failed to allocate memory");
        fclose(file);
        return NULL;
    }

    // 读取文件内容到内存
    size_t bytesRead = fread(buffer, 1, fileSize, file);
    if (bytesRead != fileSize) {
        perror("Failed to read file");
        free(buffer);
        fclose(file);
        return NULL;
    }

    // 添加空字符以结束字符串
    buffer[bytesRead] = '\0';

    // 此时，buffer中包含了文件的全部内容
    //printf("==> kfile File content:\n%s, size: %d\n", buffer, strlen(buffer));
    //AES解密
    aes_config_read(output, buffer, COMM_FILE_K_SIZE);  

     // 清理
    free(buffer);
    fclose(file);
    return output;
}

char* aes_read_cainfo(char* output)
{
    FILE *file = fopen(COMM_FILE_C, "r");
    if (file == NULL) {
        perror("Failed to open file");
        return NULL;
    }

    // 获取文件大小
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    // 分配内存以保存文件内容
    char *buffer = (char*) malloc(fileSize + 1);
    if (buffer == NULL) {
        perror("Failed to allocate memory");
        fclose(file);
        return NULL;
    }

    // 读取文件内容到内存
    size_t bytesRead = fread(buffer, 1, fileSize, file);
    if (bytesRead != fileSize) {
        perror("Failed to read file");
        free(buffer);
        fclose(file);
        return NULL;
    }

    // 添加空字符以结束字符串
    buffer[bytesRead] = '\0';

    // 此时，buffer中包含了文件的全部内容
    //printf("==> cfile File content:\n%s, size: %d\n", buffer, strlen(buffer));
    //AES解密
    aes_config_read(output, buffer, COMM_FILE_C_SIZE);  

     // 清理
    free(buffer);
    fclose(file);
    return output;
}

#ifdef AES_TEST_11
#define TEST_LEN 128
void case_test(char* input)
{
    char prase[TEST_LEN]={0};
    char* out =aes_config_write(input);
    printf("AES encrypt:%s\n", out);
    
    aes_config_read(out, prase, TEST_LEN);
    printf("AES decrypt:%s\n", prase);
    free(out);
}

int main()
{
    case_test("/app/etc/conf/data123");
    case_test("idps.db");
    case_test("/app/log/skygo");
    case_test("/app/etc/conf/CA/");
    case_test("https://qasocservice.car.360.net");
    case_test("/app/etc/conf/data123");
    case_test("/app/etc/conf/config/policy_config.json");
    return 0;
}

#endif