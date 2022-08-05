#ifndef HEADER_CONST
#define HEADER_CONST
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

/*----------------------------------*/
#define RAM 1
#define SWAP 0
#define RAM_SIZE 16
#define SWAP_SIZE 64
#define RELEASE_THRESHOLD 8
#define NUM_ITERS 10
#define ALGORITHM "NUR"
/*----------------------------------*/
long int VIRTUAL_TIME=0;
int ref_times[SWAP_SIZE] = {0};
int misses;
int addresses[SWAP_SIZE];
#endif