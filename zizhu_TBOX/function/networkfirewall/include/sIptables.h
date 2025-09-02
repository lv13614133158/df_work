#ifndef __SIPTABLES__H_
#define __SIPTABLES__H_
#ifdef __cplusplus
extern "C"{
#endif

int setIptable(char* rules);
int addIptable(char* tables, char* chains, char* sip, char *action);
int delIptable(char* tables, char* chains, int line);
int viewIptable(char* tables);

#ifdef __cplusplus
}
#endif
#endif