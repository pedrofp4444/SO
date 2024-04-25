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
  queue->size = 0;
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
  queue->size++;
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

  int highestPriority = queue->enqueue[queue->start].duration;
  int index = queue->start;

  printf("highestPriority: %d\n", highestPriority);


  for (int i = queue->start + 1; i != (queue->end + 1) % MAX_TASKS;
       i = (i + 1) % MAX_TASKS) {
    if (queue->enqueue[i].duration < highestPriority) {
      highestPriority = queue->enqueue[i].duration;
      index = i;
    }
  }
  index -=1;
  printf("index: %d\n", index);

  Task dequeuedTask = queue->enqueue[index];

  // Shift elements to the left
  for (int i = index; i != queue->start; i = (i - 1 + MAX_TASKS) % MAX_TASKS) {
    queue->enqueue[i] = queue->enqueue[(i - 1 + MAX_TASKS) % MAX_TASKS];
  }

  // Update queue indices
  queue->start = (queue->start + 1) % MAX_TASKS;
  queue->end = (queue->end - 1 + MAX_TASKS) % MAX_TASKS;

  printf("dequeuedTask: %s\n", dequeuedTask.program);
  printf("dequeuedTask duration: %d\n", dequeuedTask.duration);

  return dequeuedTask;
}

void print_queue(Queue* queue) {
  printf("Queue: \n");
  printf("start: %d\n", queue->start);
  printf("Queue end: %d\n", queue->end);
  printf("Queue size: %d\n", queue->size);

  for (int i = queue->start; i < queue->end; i++) {
    printf("Task inside  %d: %s\n", i, queue->enqueue[i].program);
  }
}

void freeQueue(Queue* queue) {
  free(queue);
}