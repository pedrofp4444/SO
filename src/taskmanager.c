#include "taskmanager.h"  // Include the header file that defines the Queue and Task types.

#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "utils.h"

Queue* createQueue() {
  Queue* queue = malloc(sizeof(Queue));
  queue->start = 0;
  queue->end = 0;
  printf("Queue created\n");
  return queue;
}

int isEmpty(Queue* queue) {
  return queue->start == queue->end;
}

void enqueue(Queue* queue, Task task) {
  if ((queue->end + 1) % MAX_TASKS == queue->start) {
    fprintf(stderr, "Queue is full. Cannot enqueue more tasks.\n");
    return;
  }
  queue->enqueue[queue->end] = task;
  queue->end = (queue->end + 1) % MAX_TASKS;
}

Task dequeue(Queue* queue) {
  if (isEmpty(queue)) {
    fprintf(stderr, "Queue is empty. Cannot dequeue.\n");
    exit(EXIT_FAILURE);
  }
  Task dequeuedTask = queue->enqueue[queue->start];
  queue->start = (queue->start + 1) % MAX_TASKS;
  return dequeuedTask;
}

Task dequeue_Priority(Queue* queue) {
  if (isEmpty(queue)) {
    fprintf(stderr, "Queue is empty. Cannot dequeue.\n");
    exit(EXIT_FAILURE);
  }
  int highestPriority = queue->enqueue[0].duration;
  int index = 0;

  for (int i = 0; i <= queue->end; i++) {
    if (queue->enqueue[i].duration < highestPriority) {
      highestPriority = queue->enqueue[i].duration;
      index = i;
    }
  }
  Task dequeuedTask = queue->enqueue[index];
  queue->start = (queue->start + 1) % MAX_TASKS;

  for (int i = index; i < queue->end; i++) {
    if (queue->enqueue[i].duration > 10) {
      queue->enqueue[i].duration -= 10;
    }
    queue->enqueue[i] = queue->enqueue[i + 1];
  }
  return dequeuedTask;
}

void print_queue(Queue* queue) {
  printf("Queue: \n");
  printf("start: %d\n", queue->start);
  printf("Queue end: %d\n", queue->end);

  for (int i = queue->start; i < queue->end; i++) {
    printf("Task inside  %d: %s\n", i, queue->enqueue[i].program);
  }
}

void freeQueue(Queue* queue) {
  free(queue);
}