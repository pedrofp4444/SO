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

#endif
