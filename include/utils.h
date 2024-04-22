#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#ifndef UTILS_H
#define UTILS_H

#define CLIENT "tmp/client_fifo"
#define SERVER "tmp/server_fifo"
#define MAX_TASKS 30

typedef struct {
  char program[308];
  int duration;
  pid_t pid;
} Task;

typedef struct {
  Task enqueue[MAX_TASKS];
} Enqueue;

#endif