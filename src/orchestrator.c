#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "utils.h"

/*
  server output 10 sjf
*/
int main(int argc, char* argv[]) {
  if (argc < 4) {
    printf("Usage: <output_folder> <parallel-tasks> <sched-policy>");
    return 1;
  }

  int fd;
  int completed_tasks = 0;
  struct timeval start, end;
  long elapsed_time;
  // char* shceduling_algorithm = argv[3];
  // char* path_of_completed_tasks = argv[1];

  completed_tasks = open(argv[1], O_CREAT | O_RDWR, 0644);

  // Create server FIFO
  createFiFO(SERVER);

  // Open server FIFO for reading CLIENT
  fd = openFiFO(SERVER, O_RDONLY);

  while (1) {
    Task task;
    int bytes_read = 0;
    while ((bytes_read = read(fd, &task, sizeof(task))) > 0) {
      // Start timer
      gettimeofday(&start, NULL);

      if (fork() == 0) {
        // Start timer
        gettimeofday(&start, NULL);
        printf("Received task_%d: %s\n", task.pid, task.program);

        // Child process

        char* instructions[100];
        parseInstructions(task.program, instructions, MAX_TASKS);

        // Open client FIFO for writing response
        char fifo_name[50];
        sprintf(fifo_name, CLIENT "_%d", task.pid);
        int fd_client = openFiFO(fifo_name, O_WRONLY);

        // Redirect stdout to a pipe
        int pipefd[2];
        if (pipe(pipefd) == -1) {
          perror("pipe");
          return 1;
        }

        if (fork() == 0) {
          // Child process to execute the task
          close(pipefd[0]);
          dup2(
              pipefd[1], STDOUT_FILENO
          );  // Redirect stdout to the write end of the pipe
          close(pipefd[1]);

          // Execute task
          executeTask(task.program, instructions);

          // End timer
          gettimeofday(&end, NULL);
        } else {
          // Parent process
          close(pipefd[1]);
          char buffer[1024];
          ssize_t nbytes = 0;

          while ((nbytes = read(pipefd[0], buffer, sizeof(buffer))) > 0) {
            // Write the output of the task back to the client
            write(fd_client, buffer, nbytes);
          }
          close(pipefd[0]);

          elapsed_time = (end.tv_sec - start.tv_sec) * 1000 +
                         (end.tv_usec - start.tv_usec) / 1000;

          char completed_tasks_msg[100];
          sprintf(
              completed_tasks_msg, "Tarefa %d executada em %ld segundos\n",
              task.pid, elapsed_time
          );

          writeFiFO(
              completed_tasks, completed_tasks_msg, strlen(completed_tasks_msg)
          );

          // Close client FIFO and exit
          closeFiFO(fd_client);

          write(STDOUT_FILENO, "Task completed\n", 15);
          _exit(0);
        }
      }
    }
  }
  return 0;
}
