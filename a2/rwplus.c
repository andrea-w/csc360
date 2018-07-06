/*Required Headers*/

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include "rw.h"
#include "resource.h"

/*
 *   This solution is based on the algorithm presented in Doeppner pp.67-68
 *   as a solution to the readers-writers problem
 */

/*
 * Declarations for reader-writer shared variables -- plus concurrency-control
 * variables -- must START here.
 */
static resource_t data;
int readers = 0;
int writers = 0;
int active_writers = 0;
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t readerQ = PTHREAD_COND_INITIALIZER;
pthread_cond_t writerQ = PTHREAD_COND_INITIALIZER;

void initialize_readers_writer() {
    /*
     * Initialize the shared structures, including those used for
     * synchronization.
     */
}


void rw_read(char *value, int len) {
    pthread_mutex_lock(&m);
    while(!(writers==0)) {
    	pthread_cond_wait(&readerQ, &m);
    }
    readers++;
    pthread_mutex_unlock(&m);
    read_resource(&data, value, len);
    pthread_mutex_lock(&m);
    if(--readers == 0) {
    	pthread_cond_signal(&writerQ);
    }
    pthread_mutex_unlock(&m);
}


void rw_write(char *value, int len) {
    pthread_mutex_lock(&m);
    writers++;
    while(!((readers == 0) && (active_writers == 0))) {
    	pthread_cond_wait(&writerQ, &m);
    }
    active_writers++;
    pthread_mutex_unlock(&m);
    write_resource(&data, value, len);
    pthread_mutex_lock(&m);
    active_writers--;
    if (--writers == 0) {
        pthread_cond_broadcast(&readerQ);	
    }
    else {
        pthread_cond_signal(&writerQ);    	
    }
    pthread_mutex_unlock(&m);
}
