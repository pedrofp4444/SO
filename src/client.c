#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "taskmanager.h"
#include "utils.h"

int main(int argc, char* argv[]) {
  // Creates the task struct to be sent to the orchestrator
  Task task;

  // Gets the pid of the client atual process
  pid_t pid = getpid();

  // Creates the client fifo from where the client will receive the task id
  char fifo_name[50];
  sprintf(fifo_name, CLIENT "_%d", pid);
  createFIFO(fifo_name);

  // Verifies the command and sets the task type
  if (strcmp(argv[1], "status") == 0) {
    // If the command is status, the task type is set to STATUS
    task.type = STATUS;

    task.id = 0;
    task.pid = pid;
    strcpy(task.program, "");
    // Verifies if the number of arguments is correct
  } else if (argc < 5) {
    // Warns the user about the correct usage
    char* usage = 
    "Usage:\n $ excute <flag [-u // -p]> <duration> <program & args>\n $ status\n";
    write(1, usage, strlen(usage));
    exit(1);
  } else {
    // If the command is execute, the task type is set to EXECUTE
    task.type = EXECUTE;
    task.id = -1;

    // Sets the task pid, duration and program
    task.pid = pid;
    task.duration = strtol(argv[3], NULL, 10);
    strcpy(task.program, argv[4]);
  }

  // Initializes the task id to -1 so that the orchestrator can set it after receiving the task

  // Opens the orchestrator fifo to send the task
  int orchestrator_fifo = openFIFO(ORCHESTRATOR, O_WRONLY);

  // Sends the task to the orchestrator
  write(orchestrator_fifo, &task, sizeof(task));

  // Opens the client fifo to receive the task id
  int fd_client = openFIFO(fifo_name, O_RDONLY);

  // Reads the task id from the client fifo

  Status task_status;
  if (read(fd_client, &task, sizeof(task)) > 0) {
    if (task.type == STATUS) {
      int x = 1;
      while (x) {
        if (read(fd_client, &task_status, sizeof(Status)) > 0) {
          pretier_print_status(task_status);
          x = 0;
        }
      }

    } else {
      write_id(task.id);
      // printf("Task id: %d\n", task.id);
    }
  }

  // Closes the client fifo
  close(orchestrator_fifo);

  return 0;
}
