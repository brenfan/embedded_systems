#ifndef __SORT_H__
#define __SORT_H__

#include <pthread.h>

void sort_start(void);
void sort_stop(void);
int sort_getArrayLength(void);
void sort_setArrayLength(int a);
int sort_getArraysSorted(void);
int sort_getArrayIndex(int i);
int *sort_getArray(void);
pthread_mutex_t *sort_getLock(void);

int sort_isRunning(void);
void sort_init(void);
void sort_destroy(void);
#endif
