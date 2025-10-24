#ifndef __OEPN_AES_H__
#define __OEPN_AES_H__
#ifdef __cplusplus
extern "C" {
#endif

// 输入一段字符串 返回加密后的字符串 ,返回值需手动释放
char* aes_char_write(char* input);
// 输入一段加密的字符串，返回解密的字符串，返回值需要手动释放
char* aes_char_read(char* input, int max_len);

//  初始化aes 设置key iv 若不初始化则使用默认值 
int aes_init(char*key, char *iv);
// 如使用aes_init  则需要调用此函数释放内存
void aes_delete();
#ifdef __cplusplus
}
#endif
#endif