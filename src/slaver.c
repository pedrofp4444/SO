#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "taskmanager.h"
#include "utils.h"

void slaver() {
  int slaver_fd = openFiFO(SLAVER, O_RDONLY);

  if (slaver_fd == 0) {
    return;
  }

  char buffer[308];
  readFiFO(slaver_fd, buffer, sizeof(buffer));

  char* instructions[MAX_TASKS];
  parseInstructions(buffer, instructions);

  executeTask(instructions);

  closeFiFO(slaver_fd);
}