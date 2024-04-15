#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define FIFO_NAME "task_fifo"

int main() {
  int fd;
  char task[256];  // It is needed to minimize the buffer size

  fd = open(FIFO_NAME, O_WRONLY);

  strcpy(task, "Example of task from client");
  write(fd, task, strlen(task) + 1);

  close(fd);

  return 0;
}
