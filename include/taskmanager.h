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
#define MAX_TASKS 7

typedef struct {
  Task enqueue[MAX_TASKS];
  int start, end;
  int size;
} Queue;

Queue* createQueue();

int isEmpty(Queue* queue);

void enqueue(Queue* queue, Task task);

Task dequeue(Queue* queue);

Task dequeue_Priority(Queue* queue);

void print_queue(Queue* queue);

void freeQueue(Queue* queue);

#endif