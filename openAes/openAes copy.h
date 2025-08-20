#ifndef __OEPN_AES_H__
#define __OEPN_AES_H__
#ifdef __cplusplus
extern "C" {
#endif

// 先AES cbc128加密再base64,返回值需手动释放
char* aes_config_write(char* input);
// 先反base64 再AES cbc128解密，,返回值需手动释放
char* aes_config_read(char* output, char* input, int len);

char* aes_read_sslcert(char* output);
char* aes_read_sslkey(char* output);
char* aes_read_cainfo(char* output);


#ifdef __cplusplus
}
#endif
#endif