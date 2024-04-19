#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define SERVER "server_fifo"
#define CLIENT "client_fifo"

typedef struct {
  char program[256];
  int duration;
  pid_t pid;
} Task;

int main() {
  int fd;
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
      if (fork() == 0) {
        // Child process
        printf(
            "Task received: %s %d %d\n", task.program, task.duration, task.pid
        );

        // Open client FIFO for writing response
        char fifo_name[50];
        sprintf(fifo_name, "%s_%d", CLIENT, task.pid);
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
          execlp(task.program, task.program, NULL);
          perror("execlp");
          exit(1);
        } else {
          // Parent process
          close(pipefd[1]);
          char buffer[1024];
          ssize_t nbytes;
          while ((nbytes = read(pipefd[0], buffer, sizeof(buffer))) > 0) {
            // Write the output of the task back to the client
            write(fd_client, buffer, nbytes);
          }
          close(pipefd[0]);

          // Close client FIFO and exit
          close(fd_client);
          _exit(0);
        }
      }
    }
  }
  return 0;
}
