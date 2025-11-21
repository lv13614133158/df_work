#ifndef __CLOCKER_H
#define __CLOCKER_H
#ifdef __cplusplus
extern "C" {
#endif

//sync system time
void sync_clock(long long time);

//get timestamp
long long get_current_time();

#ifdef __cplusplus
}  /* end of the 'extern "C"' block */
#endif
#endif
