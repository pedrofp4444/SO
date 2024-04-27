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

  int min = queue->enqueue[queue->start].duration;
  Task minTask = queue->enqueue[queue->start];
  int index = queue->start;

  for (int i = queue->start; i < queue->end; i++) {
    if (queue->enqueue[i].duration != -1 && queue->enqueue[i].duration < min) {
      min = queue->enqueue[i].duration;
      minTask = queue->enqueue[i];
      index = i;
    }
    if (queue->enqueue[i].duration > 10) {
      queue->enqueue[i].duration -= 10;
    }
  }
  printf("index: %d\n", index);

  for (int i = index; i < queue->end - 1; i++) {
    queue->enqueue[i] = queue->enqueue[i + 1];
  }
  queue->enqueue[queue->end - 1].duration = -1;

  return minTask;
}

void print_queue(Queue* queue) {
  printf("Queue: \n");
  printf("start: %d\n", queue->start);
  printf("Queue end: %d\n", queue->end);
  printf("Queue size: %d\n", queue->size);

  for (int i = queue->start; i < queue->end; i++) {
    printf(
        "Task inside  %d: %d(mseg) %s\n", i, queue->enqueue[i].duration,
        queue->enqueue[i].program
    );
  }
}

void freeQueue(Queue* queue) {
  free(queue);
}

int available_slave(int parallel_tasks[]) {
  for (int i = 0; i < sizeof(parallel_tasks); i++) {
    if (parallel_tasks[i] == 0) {
      parallel_tasks[i] = 1;
      return i;
    }
  }
  return -1;
}