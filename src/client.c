#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "argument_parser.c"

#define FIFO_NAME "task_fifo"

int main(int argc, char *argv[]) {
  // Testing the parser
  TaskInfo task_info = parse_arguments(argc, argv);

  printf("Time: %d\n", task_info.time);
  printf("Program: %s\n", task_info.program);
  printf("Args: %s\n", task_info.args);
  // ------

  int fd;
  char task[256];  // It is needed to minimize the buffer size

  fd = open(FIFO_NAME, O_WRONLY);

  strcpy(task, "Example of task from client");
  write(fd, task, strlen(task) + 1);

  close(fd);

  return 0;
}
