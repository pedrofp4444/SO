#include "taskmanager.h"

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
  // Allocates memory for the queue
  Queue* queue = malloc(sizeof(Queue));

  // Initializes the queue
  queue->start = 0;
  queue->end = 0;
  queue->size = 0;

  return queue;
}

int isEmpty(Queue* queue) {
  // Verifies if the queue is empty
  if (queue->enqueue[queue->start].duration == -1) {
    return 1;
  }
  return queue->start == queue->end;
}

void enqueue(Queue* queue, Task task) {
  // Verifies if the queue is full
  if ((queue->end + 1) % MAX_TASKS == queue->start) {
    fprintf(stderr, "Queue is full. Cannot enqueue more tasks.\n");
    return;
  }
  queue->enqueue[queue->end] = task;
  queue->end = (queue->end + 1) % MAX_TASKS;
  queue->size++;
}

Task dequeue(Queue* queue) {
  // Verifies if the queue is empty
  if (isEmpty(queue)) {
    fprintf(stderr, "Queue is empty. Cannot dequeue.\n");
    exit(EXIT_FAILURE);
  }

  // Dequeues the next task
  Task dequeuedTask = queue->enqueue[queue->start];
  queue->start = (queue->start + 1) % MAX_TASKS;
  return dequeuedTask;
}

Task dequeue_with_priority(Queue* queue) {
  // Verifies if the queue is empty
  if (isEmpty(queue)) {
    fprintf(stderr, "Queue is empty. Cannot dequeue.\n");
    exit(EXIT_FAILURE);
  }

  // Dequeues the task with the highest priority by comparing the duration of the tasks

  // Initializes the minimum duration with the first task in the queue
  int min = queue->enqueue[queue->start].duration;
  Task minTask = queue->enqueue[queue->start];
  int index = queue->start;

  // Iterates over the queue to find the task with the minimum duration
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

  // Removes the task from the queue by shifting the tasks to the left
  for (int i = index; i < queue->end - 1; i++) {
    queue->enqueue[i] = queue->enqueue[i + 1];
  }
  queue->enqueue[queue->end - 1].duration = -1;

  return minTask;
}

void print_queue(Queue* queue) {
  // Prints the queue of tasks
  printf("Queue: \n");
  printf("start: %d\n", queue->start);
  printf("Queue end: %d\n", queue->end);
  printf("Queue size: %d\n", queue->size);

  // Iterates over the queue to print the tasks
  for (int i = queue->start; i < queue->end; i++) {
    printf(
      "Task inside  %d: %d(mseg) %s\n", i, queue->enqueue[i].duration,
      queue->enqueue[i].program
    );
  }
}

void freeQueue(Queue* queue) {
  // Frees the queue
  free(queue);
}

Status* createStatus() {
  // Allocates memory for the status
  Status* status = malloc(sizeof(Status));
  status->end = 0;

  return status;
}

METRICS createMetrics(int id, char* program) {
  // Creates a metrics
  Type type = SCHEDULED;
  METRICS metrics;
  metrics.id = id;

  strcpy(metrics.program, program);
  metrics.type = type;

  return metrics;
}

void enqueueStatus(Status* status, METRICS metrics) {
  // Verifies if the status is full
  if (status->end == MAX_TASKS) {
    fprintf(stderr, "Status is full. Cannot enqueue more metrics.\n");
    return;
  }
  status->metrics[status->end] = metrics;
  status->end++;
}

void changeMETRICS(Status* status, int id, Phase type) {
  // Iterates over the status to find the metrics with the given id
  for (int i = 0; i < status->end; i++) {
    if (status->metrics[i].id == id) {
      // Changes the type of the metrics
      status->metrics[i].type = type;
      return;
    }
  }
}

void print_status(Status* status) {
  // Prints the status of the tasks
  printf("Status: \n");

  printf("Status end: %d\n", status->end);

  // Iterates over the status to print the metrics
  for (int i = 0; i < status->end; i++) {
    printf(
      "Metrics inside %d: %d %d\n", i, status->metrics[i].id,
      status->metrics[i].type
    );
  }
}

void pretier_print_status(Status status) {
  // Prints the status of the tasks
  char COMPLETED[] = "--[ COMPLETED ]--\n";
  write(1, COMPLETED, strlen(COMPLETED));

  for (int i = 0; i < status.end; i++) {
    char task[500];
    if (status.metrics[i].type == 2) {
      sprintf(task, "Task %d -- %s\n", status.metrics[i].id, status.metrics[i].program);
      write(1, task, strlen(task));
    }
  }

  char EXECUTING[] = "--[ EXECUTING ]--\n";
  write(1, EXECUTING, strlen(EXECUTING));

  for (int i = 0; i < status.end; i++) {
    char task[500];
    if (status.metrics[i].type == 0) {
      sprintf(task, "Task %d -- %s\n", status.metrics[i].id, status.metrics[i].program);
      write(1, task, strlen(task));
    }
  }

  char SCHEDULED[] = "--[ SCHEDULED ]--\n";
  write(1, SCHEDULED, strlen(SCHEDULED));

  for (int i = 0; i < status.end; i++) {
    char task[500];
    if (status.metrics[i].type == 1) {
      sprintf(task, "Task %d -- %s\n", status.metrics[i].id, status.metrics[i].program);
      write(1, task, strlen(task));
    }
  }


}