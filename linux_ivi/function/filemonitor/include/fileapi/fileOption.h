#ifndef _FILEOPTION_H_
#define _FILEOPTION_H_
#include <stdbool.h>

int cmd_cp(char *sourcePath,char *destPath);
int get_ls_return_length(char *dirname);
int cmd_ls(char *dirname,char *out);
int cmd_mv(const char *oldpath,const char *newpath);
int cmd_rm(const char *path);
int cmd_chown(const char *userAndGroup,const char *filePath);
int cmd_chmod(int mode ,const char *filePath);
void mode_to_letters(int mode,char str[]);

#endif
