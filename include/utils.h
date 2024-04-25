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
#define MAX_TASKS 7

typedef struct {
  char program[308];
  int duration;
  pid_t pid;
} Task;

int createFiFO(char* name);

int openFiFO(char* name, int mode);

void writeFiFO(int fd, void* data, size_t size);

void readFiFO(int fd, void* data, size_t size);

void closeFiFO(int fd);

void parseInstructions(char* program, char* instructions[]);

void executeTask(char* instructions[]);

void redirectStdout(int pipefd[2]);

void redirectStdin(int pipefd[2]);

#endif