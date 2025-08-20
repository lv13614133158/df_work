#ifndef __PROCESSCHECK_H
#define __PROCESSCHECK_H
#include "util.h"


char *get_process_information();
void processCheckInit(list *listName);
void checkProcessCloud();
void checkProcessLocal();
void checkProcessChange();
void checkProcessList();
void checkZombieProcess();
#endif