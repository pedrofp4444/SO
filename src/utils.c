#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_COMMANDS 10
#define PATH_MAX 4096

int createFiFO(char* name) {
  int result = mkfifo(name, 0644);
  if (result < 0) {
    perror("Error creating FIFO");
    return 0;
  }
  return 1;
}

int openFiFO(char* name, int mode) {
  int result = open(name, mode);
  if (result < 0) {
    perror("Error opening FIFO");
    return 0;
  }
  return result;
}

void writeFiFO(int fd, void* data, size_t size) {
  write(fd, data, size);
}

void readFiFO(int fd, void* data, size_t size) {
  read(fd, data, size);
}

void closeFiFO(int fd) {
  close(fd);
}

void parseInstructions(char* program, char* instructions[]) {
  char* token;
  int i = 0;

  // Use a copy of the program string since strsep modifies the original string
  char* program_copy = strdup(program);

  if (program_copy == NULL) {
    perror("Error duplicating string");
    exit(1);
  }

  // Tokenize the program string
  while ((token = strsep(&program_copy, " ")) != NULL) {
    instructions[i] = token;
    i++;
  }

  // Null-terminate the instructions array
  instructions[i] = NULL;

  // Free the duplicated string
  free(program_copy);
}

void executeTask(char* instructions[]) {
  execvp(instructions[0], instructions);
  perror("execvp");
  exit(1);
}

void redirectStdout(int pipefd[2]) {
  if (pipe(pipefd) == -1) {
    perror("pipe");
    exit(1);
  }

  if (fork() == 0) {
    close(pipefd[0]);
    dup2(pipefd[1], STDOUT_FILENO);
  }
}

void redirectStdin(int pipefd[2]) {
  if (pipe(pipefd) == -1) {
    perror("pipe");
    exit(1);
  }

  if (fork() == 0) {
    close(pipefd[1]);
    dup2(pipefd[0], STDIN_FILENO);
  }
}
