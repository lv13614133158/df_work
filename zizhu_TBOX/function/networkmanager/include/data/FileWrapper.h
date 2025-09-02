#ifndef __FILEWRAPPER_H
#define __FILEWRAPPER_H
#ifdef __cplusplus
extern "C"{
#endif
typedef struct FileWrapper{
    char* fileData;
    long long fileDataLen;
    char* errorMsg;
}FileWrapper_t;
#ifdef __cplusplus
}
#endif
FileWrapper_t *processFileWrapper(char* path);
#endif //__FILEWRAPPER_H