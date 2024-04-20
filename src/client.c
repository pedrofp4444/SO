#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define CLIENT "tmp/client_fifo"
#define SERVER "tmp/server_fifo"

#define ASK "Enter the task (format: execute <duration> <program>):\n"

typedef struct {
  char program[256];
  int duration;
  pid_t pid;
} Task;

int main(int argc, char* argv[]) {
  char input[256];
  int fd_S, fd_c;

  write(STDOUT_FILENO, ASK, strlen(ASK));

  ssize_t bytes_read = 0;
  while ((bytes_read = read(STDIN_FILENO, input, sizeof(input))) > 0) {
    Task task;
    char fifo_name[50];
    if (fork() == 0) {
      task.pid = getpid();
      if (sscanf(input, "execute %d %s", &task.duration, task.program) == 2) {
        sprintf(fifo_name, CLIENT "_%d", task.pid);

        // Create client FIFO

        if (mkfifo(fifo_name, 0644) < 0) {
          perror("Error creating FIFO");
          exit(1);
        }

        // Open server FIFO

        fd_S = open(SERVER, O_WRONLY);
        if (fd_S < 0) {
          perror("Error opening server FIFO for writing");
          exit(1);
        }

        // Write task to server
        write(fd_S, &task, sizeof(task));

      } else {
        printf(
            "Invalid input format. Please use format: execute <duration> <program>\n"
        );
      }

      // Open client FIFO for reading
      sprintf(fifo_name, CLIENT "_%d", getpid());
      printf("fifo_name: %s\n", fifo_name);

      if ((fd_c = open(fifo_name, O_RDONLY)) < 0) {
        perror("Error opening FIFO");
        exit(1);
      }
      // Read from client FIFO and write to stdout
      char buffer[1024];
      ssize_t bytes_read;
      while ((bytes_read = read(fd_c, buffer, sizeof(buffer))) > 0) {
        write(STDOUT_FILENO, buffer, bytes_read);
      }
      close(fd_c);
      _exit(0);

    } else {
      wait(NULL);
      write(STDOUT_FILENO, ASK, strlen(ASK));
    }
  }
  close(fd_S);
  return 0;
}
