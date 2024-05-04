#ifndef TASKMANAGER_H
#define TASKMANAGER_H

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "utils.h"

// Defines the maximum number of tasks to be in the queue
#define MAX_TASKS 30

typedef struct {
  Task metrics[MAX_TASKS];
  int end;

} Status;

// Struct to represent a queue of tasks
typedef struct {
  Task enqueue[MAX_TASKS];
  int start, end;
  int size;
} Queue;

/**
 * Creates a queue of tasks
 * 
 * @return The queue created
 */
Queue* createQueue();

/**
 * Verifies if the queue is empty
 * 
 * @param queue The queue to be verified
 * @return 1 if the queue is empty, 0 otherwise
 */
int isEmpty(Queue* queue);

/**
 * Enqueues a task in the queue
 * 
 * @param queue The queue to enqueue the task
 * @param task The task to be enqueued
 */
void enqueue(Queue* queue, Task task);

/**
 * Dequeues a task from the queue
 * 
 * @param queue The queue to dequeue the task
 * @return The dequeued task
 */
Task dequeue(Queue* queue);

/**
 * Dequeues a task from the queue with priority
 * 
 * @param queue The queue to dequeue the task
 * @return The dequeued task
 */
Task dequeue_with_priority(Queue* queue);

/**
 * Prints the queue of tasks
 * 
 * @param queue The queue to be printed
 */
void print_queue(Queue* queue);

/**
 * Frees the queue of tasks
 * 
 * @param queue The queue to be freed
 */
void freeQueue(Queue* queue);

/**
 * Creates a status of the tasks
 * 
 * @return The status created
*/
Status* createStatus();


/**
 * Enqueues a status in the status
 * 
 * @param status The status to enqueue
 * @param task The metrics to enqueue
*/
void enqueueStatus(Status* status, Task task);

/**
 * Updates the status of the tasks
 * 
 * @param status The status to update
 * @param task The metrics to update
*/
void updateStatus(Status* status, Task task);

/**
 * Finds a task in the status by its id
 * 
 * @param status The status to find the task
 * @param id The id of the task
 * 
 * @return The task found
*/
Task findTask(Status* status, int id);



/**
 * Prints the status of the tasks
 * 
*/
void print_status(Status* status);

/**
 * Prints the status of the tasks
 * 
*/
void pretier_print_status(Status *status);



#endif
