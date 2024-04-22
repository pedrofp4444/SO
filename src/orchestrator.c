#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <utils.h>

/*
  server output 10 sjf
*/
int main(int argc, char* argv[]) {
  if (argc < 4) {
    printf("Usage: %s <path_of_completed_tasks>\n", argv[0]);
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
  if (mkfifo(SERVER, 0644) < 0) {
    perror("Error creating server FIFO");
    return 1;
  }

  // Open server FIFO for reading CLIENT
  if ((fd = open(SERVER, O_RDONLY)) < 0) {
    perror("Error opening server FIFO for reading");
    return 1;
  }

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

        char* intructions[100];
        char* token = strtok(task.program, " ");
        int i = 0;
        while (token != NULL) {
          intructions[i] = token;
          token = strtok(NULL, " ");
          i++;
        }
        intructions[i++] = NULL;

        // Open client FIFO for writing response
        char fifo_name[50];
        sprintf(fifo_name, CLIENT "_%d", task.pid);
        int fd_client = open(fifo_name, O_WRONLY);
        if (fd_client < 0) {
          perror("Error opening client FIFO for writing response");
          return 1;
        }

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
          execvp(intructions[0], intructions);

          // End timer
          gettimeofday(&end, NULL);

          perror("execvp");
          exit(1);
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
          write(
              completed_tasks, completed_tasks_msg, strlen(completed_tasks_msg)
          );

          // Close client FIFO and exit
          close(fd_client);

          write(STDOUT_FILENO, "Task completed\n", 15);
          _exit(0);
        }
      }
    }
  }
  return 0;
}
