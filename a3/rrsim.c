#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "linkedlist.h"

#define MAX_BUFFER_LEN 80

taskval_t *event_list = NULL;

void print_task(taskval_t *t, void *arg) {
    printf("task id= %03d arrival time=%5d req=%3.2f used=%3.2f\n",
        t->id,
        t->arrival_time,
        t->cpu_request,
        t->cpu_used
    );  
}


void increment_count(taskval_t *t, void *arg) {
    int *ip;
    ip = (int *)arg;
    (*ip)++;
}

void run_simulation(int qlen, int dlen) {
    taskval_t *ready_q = NULL;
    int tick = 0;
    int quantum_counter = 0;
    int dispatch_counter = 0;

    while(1) {
        taskval_t *front_event = peek_front(event_list);
        taskval_t *front_ready = peek_front(ready_q);

        /* if event_list and ready_q are both empty, there are no more tasks to run
           so exit the program */
        if (front_event == NULL && front_ready == NULL) {
            break;
        }

        /* if the first node of event_list has arrived, add it to the ready_q */
        if (front_event != NULL && tick == front_event->arrival_time) {
            event_list = remove_front(event_list);
            ready_q = add_end(ready_q, front_event);
        }

        /* if there exists a node at the front of the ready_q */
        while(1) {
            front_ready = peek_front(ready_q);
            front_event = peek_front(event_list);

            if (front_ready != NULL) {
                /* if dispatch is not necessary */
                if (dispatch_counter == dlen) {
                    /* if task is incomplete but have reached quantum max, add event to end of ready_q */
                    if (quantum_counter == qlen && front_ready->cpu_used  < front_ready->cpu_request) {
                        ready_q = remove_front(ready_q);
                        ready_q = add_end(ready_q, front_ready);
                        dispatch_counter = 0;
                        quantum_counter = 0;
                    }
                    /* else if task is completed, end task and print task statement */
                    else if (front_ready->cpu_used >= front_ready->cpu_request) {
                        float turnaround_time = tick - front_ready->arrival_time;
                        float wait = turnaround_time - front_ready->cpu_request;
                        ready_q = remove_front(ready_q);
                        printf("[%05d] id=%05d EXIT w=%3.2f ta=%3.2f\n", tick, front_ready->id, wait, turnaround_time);
                        end_task(front_ready);
                        dispatch_counter = 0;
                        quantum_counter = 0;
                    }
                    /* else run the task */
                    else {
                        printf("[%05d] id=%05d req=%3.2f used=%3.2f\n", tick, front_ready->id, front_ready->cpu_request, front_ready->cpu_used);
                        front_ready->cpu_used = front_ready->cpu_used + 1;
                        quantum_counter++;
                        break;
                    }
                }
                /* else dispatch */
                else {
                    printf("[%05d] DISPATCHING\n", tick);
                    ++dispatch_counter;
                    break;
                }
            }
            /* else IDLE */
            else {
                if ( (front_event = peek_front(event_list)) != NULL) {
                    printf("[%05d] IDLE\n", tick);
                    break;
                }
                else {
                    exit(EXIT_SUCCESS);
                }
            }
        } 

        if ( ((front_event = peek_front(event_list)) == NULL) && ((front_ready = peek_front(ready_q)) == NULL) ) {
            break;
        }   

        ++tick;
    }                                  
}


int main(int argc, char *argv[]) {
    char   input_line[MAX_BUFFER_LEN];
    int    i;
    int    task_num;
    int    task_arrival;
    float  task_cpu;
    int    quantum_length = -1;
    int    dispatch_length = -1;

    taskval_t *temp_task;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--quantum") == 0 && i+1 < argc) {
            quantum_length = atoi(argv[i+1]);
        }
        else if (strcmp(argv[i], "--dispatch") == 0 && i+1 < argc) {
            dispatch_length = atoi(argv[i+1]);
        }
    }

    if (quantum_length == -1 || dispatch_length == -1) {
        fprintf(stderr, 
            "usage: %s --quantum <num> --dispatch <num>\n",
            argv[0]);
        exit(1);
    }


    while(fgets(input_line, MAX_BUFFER_LEN, stdin)) {
        sscanf(input_line, "%d %d %f", &task_num, &task_arrival,
            &task_cpu);
        temp_task = new_task();
        temp_task->id = task_num;
        temp_task->arrival_time = task_arrival;
        temp_task->cpu_request = task_cpu;
        temp_task->cpu_used = 0.0;
        event_list = add_end(event_list, temp_task);
    }

#ifdef DEBUG
    int num_events = 0;
    apply(event_list, increment_count, &num_events);
    printf("DEBUG: # of events read into list -- %d\n", num_events);
    printf("DEBUG: value of quantum length -- %d\n", quantum_length);
    printf("DEBUG: value of dispatch length -- %d\n", dispatch_length);
#endif

    run_simulation(quantum_length, dispatch_length);

    return (0);
}
