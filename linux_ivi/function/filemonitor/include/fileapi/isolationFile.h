
#ifndef __ISOLATIONFILE_H
#define __ISOLATIONFILE_H

typedef void (*spaceListener)(int);// 参数int 1表示隔离目录超出阈值，2表示备份目录超过阈值

int setFileWorkDir(const char *workDir);

/**
 *  @func: 设置隔离空间阈值
 *  @param: threshold,单位MB
 */
void setOfflimitDirectory(int threshold);

void IsolateFile(const char *filePath);

void restoreFile(int mode,const char *filePath);//恢复文件，mode=1，表示恢复隔离目录，mode=2表示恢复备份目录

void setBackupDirectory(const int threshold);
void backupFile(char *filePath);

int registerInsufficientSpaceEventListener(spaceListener obj);
void unRegisterInsufficientSpaceEventListener(int id);

#endif