#ifndef _CIPHER_HMAC_ALL_H
#define _CIPHER_HMAC_ALL_H
#ifdef  __cplusplus
extern "C" {
#endif /* __cplusplus */

#define    MD5_DIGEST_SIZE  16
void hmac_md5(unsigned char *key, int key_len,unsigned char *text, int text_len, unsigned char *hmac);
//md5_str用来存放摘要，最好是33字节数组,
//Compute_xxx输出是转换为字符串，get_xxx输出是原始数据(部分无法打印显示)
int Compute_string_md5(const char *dest_str, unsigned int dest_len, char *md5_str);
int Compute_file_md5(const char *file_path, char *md5_str);  
int Compute_file_sha1(const char *filePath,unsigned char *hash_out);
int get_string_sha1(const char *dest_str, unsigned int dest_len, unsigned char *sha1_str);

#ifdef  __cplusplus
}
#endif /* __cplusplus */
#endif /* _CIPHER_HMAC_ALL_H */
