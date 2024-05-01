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
#define MAX_COMMANDS 10
#define PATH_MAX 4096

typedef enum type {
  STATUS,
  EXECUTE
} Type;

typedef struct {
  Type type;
  char program[308];
  int duration;
  int id;
  pid_t pid;
  struct timeval start_time;
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

int exec_command(char* arg);

int execute_task(int number_of_commands, char** commands, char* output_file);

int count_commands(char* program);

void split_commands(char* program, char** task_commands, int number_of_commands);

#endif
