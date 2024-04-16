#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  int time;
  char program[256];
  char args[256];
} TaskInfo;

TaskInfo parse_arguments(int argc, char *argv[]) {
  TaskInfo task_info = {0};

  if (argc < 5) {
    fprintf(stderr, "Usage: %s execute time -u \"prog-a [args]\"\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  if (strcmp(argv[1], "execute") == 0 && strcmp(argv[3], "-u") == 0) {
    task_info.time = atoi(argv[2]);
    strcpy(task_info.program, argv[4]);
    strcpy(task_info.args, "");
    for (int i = 5; i < argc; i++) {
      strcat(task_info.args, argv[i]);
      strcat(task_info.args, " ");
    }
  } else {
    fprintf(stderr, "Invalid arguments\n");
    exit(EXIT_FAILURE);
  }

  return task_info;
}
