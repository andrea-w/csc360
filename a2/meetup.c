/*Required Headers*/

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include "meetup.h"
#include "resource.h"

/*
 *   This solution is based on the Reusable Barrier solution presented on p.41 of
 *   The Little Book of Semaphores
 *   http://greenteapress.com/semaphores/LittleBookOfSemaphores.pdf
 */


/*
 * Declarations for barrier shared variables -- plus concurrency-control
 * variables -- must START here.
 */
resource_t codeword;
int _count;
int _n;
int _mf;

sem_t turnstile1;
sem_t turnstile2;
sem_t barrier;
sem_t mutex; // protects resource_t codeword and int _count

void initialize_meetup(int n, int mf) {
    char label[100];

    if (n < 1) {
        fprintf(stderr, "Who are you kidding?\n");
        fprintf(stderr, "A meetup size of %d??\n", n);
        exit(1);
    }

    /*
     * Initialize the shared structures, including those used for
     * synchronization.
     */
    init_resource(&codeword, "init-codeword");

    _n = n;
    _mf = mf;
    _count = 0;

    if (sem_init(&turnstile1, 0, 0) != 0) {
        fprintf(stderr, "Error initializing semaphore 'turnstile1'. Exiting...\n");
        exit(EXIT_FAILURE);
    }
    if (sem_init(&turnstile2, 0, 1) != 0) {
        fprintf(stderr, "Error initializing semaphore 'turnstile2'. Exiting...\n");
        exit(EXIT_FAILURE);
    }
    if (sem_init(&barrier, 0, _n) != 0) {
        fprintf(stderr, "Error initializing semaphore 'barrier'. Exiting...\n");
        exit(EXIT_FAILURE);
    }
    if (sem_init(&mutex, 0, 1) != 0) {
        fprintf(stderr, "Error initializing semaphore 'mutex'. Exiting...\n");
        exit(EXIT_FAILURE);
    }
}


void join_meetup(char *value, int len) {
    fprintf(stdout, "%s has arrived\n", value);
    sem_wait(&barrier);   
    sem_wait(&mutex);

    _count++;
    if (_count == 1) {
        if (_mf == MEET_FIRST) {
            write_resource(&codeword, value, len);
            fprintf(stdout, "write value: %s\n", value);
        }
    }
    if (_count == _n) {
        if (_mf == MEET_LAST) {
            write_resource(&codeword, value, len);
            fprintf(stdout, "write value: %s\n", value);
        }
        sem_wait(&turnstile2);
        sem_post(&turnstile1);
    }
    sem_post(&mutex);

    sem_wait(&turnstile1);
    sem_post(&turnstile1);

    sem_wait(&mutex);
    _count--;
    read_resource(&codeword, value, len);
    fprintf(stdout, "read value: %s\n", value);
    if (_count == 0) {
        sem_wait(&turnstile1);
        sem_post(&turnstile2);
    }
    sem_post(&mutex);
    
    sem_wait(&turnstile2);
    sem_post(&turnstile2);

    sem_post(&barrier);

}
