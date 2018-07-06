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

// writer-priority constructs
static resource_t data;
int readers = 0;
int writers = 0;
int active_writers = 0;
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t readerQ = PTHREAD_COND_INITIALIZER;
pthread_cond_t writerQ = PTHREAD_COND_INITIALIZER;

// reader-priority constructs
static resource_t data_rp;
int readers_rp = 0;
int writers_rp = 0;
pthread_mutex_t m_rp = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t readerQ_rp = PTHREAD_COND_INITIALIZER;
pthread_cond_t writerQ_rp = PTHREAD_COND_INITIALIZER;

void initialize_readers_writer() {
    /*
     * Initialize the shared structures, including those used for
     * synchronization.
     */
}

void rw_read_rp(char *value, int len) {
    pthread_mutex_lock(&m_rp);
    while(!(writers_rp == 0)) {
        pthread_cond_wait(&readerQ_rp, &m_rp);
    }
    readers_rp++;
    pthread_mutex_unlock(&m_rp);
    read_resource(&data_rp, value, len);
    printf("RP: Read '%s' from resource.\n", value);
    pthread_mutex_lock(&m_rp);
    if (--readers_rp == 0) {
        pthread_cond_signal(&writerQ_rp);
    }
    pthread_mutex_unlock(&m_rp);
}

void rw_read(char *value, int len) {
    // first spawn child process to call rw_read_rp
    int pid;
    if ( (pid = fork()) == 0) {
        rw_read_rp(value, len);
        exit(EXIT_SUCCESS);
    }

    // then read as normal
    pthread_mutex_lock(&m);
    while(!(writers==0)) {
    	pthread_cond_wait(&readerQ, &m);
    }
    readers++;
    pthread_mutex_unlock(&m);
    read_resource(&data, value, len);
    printf("WP: Read '%s' from resource.\n", value);
    pthread_mutex_lock(&m);
    if(--readers == 0) {
    	pthread_cond_signal(&writerQ);
    }
    pthread_mutex_unlock(&m);
}

void rw_write_rp(char *value, int len) {
    pthread_mutex_lock(&m_rp);
    while(!((readers_rp == 0) && (writers_rp == 0))) {
        pthread_cond_wait(&writerQ_rp, &m_rp);
    }
    writers_rp++;
    pthread_mutex_unlock(&m_rp);
    write_resource(&data_rp, value, len);
    printf("RP: Wrote '%s' to resource.\n", value);
    pthread_mutex_lock(&m_rp);
    writers_rp--;
    pthread_cond_signal(&writerQ_rp);
    pthread_cond_broadcast(&readerQ_rp);
    pthread_mutex_unlock(&m_rp);
}

void rw_write(char *value, int len) {
    // first spawn child process to call rw_write_rp
    int pid;
    int status;
    if ( (pid = fork()) == 0) {
        rw_write_rp(value, len);
        exit(EXIT_SUCCESS);
    }

    // then write as normal
    pthread_mutex_lock(&m);
    writers++;
    while(!((readers == 0) && (active_writers == 0))) {
    	pthread_cond_wait(&writerQ, &m);
    }
    active_writers++;
    pthread_mutex_unlock(&m);
    write_resource(&data, value, len);
    printf("WP: Wrote '%s' to resource.\n", value);
    pthread_mutex_lock(&m);
    active_writers--;
    if (--writers == 0) {
        pthread_cond_broadcast(&readerQ);	
    }
    else {
        pthread_cond_signal(&writerQ);    	
    }
    pthread_mutex_unlock(&m);

    while (wait(&status) > 0); 
    
}

