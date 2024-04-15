#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define FIFO_NAME "task_fifo"
#define MAX_TASKS 100

// Still not sure of the structure of the task, so i letf a suggestion
typedef struct {
  int id;
  char program[256];  // Adapt size according to our needs
  int duration;
  pid_t pid;
} Task;

int main() {
  int fd;
  char buffer[300];  // It is needed to minimize the buffer size
  Task tasks[MAX_TASKS];
  int tasks_count = 0;

  mkfifo(FIFO_NAME, 0666);

  fd = open(FIFO_NAME, O_RDONLY);
  if (fd == -1) {
    perror("Error opening FIFO for reading");
    exit(EXIT_FAILURE);
  }

  while (read(fd, buffer, sizeof(buffer)) > 0) {
    // TODO: it is necessary to define among us the way to manage and resolve the tasks
    printf("Received task: %s\n", buffer);
  }

  close(fd);

  return 0;
}
