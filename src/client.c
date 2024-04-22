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
  int size;
  int duration;
  pid_t pid;
} Task;

int main(int argc, char* argv[]) {
  int fd_servidor, fd_cliente;


  if (argc < 2) {
    write(
      STDOUT_FILENO,
      "Invalid input format. Please use format: execute <duration> <program>\n",
      71
    );
    exit(0);
  }

  if (fork() == 0) {
    Task task;
    task.duration = atoi(argv[2]);  // Convert duration argument to integer
    task.size = argc - 3;           // Calculate the number of program arguments
    task.pid = getpid();

    // Check if memory allocation is successful
    if (task.program == NULL) {
      perror("Memory allocation failed");
      exit(EXIT_FAILURE);
    }
    // Copy program arguments to task.program

    for (int i = 3; i < argc; i++) {
      strcat(task.program, argv[i]);
      if (i != argc - 1) {  // Add space if not the last argument
        strcat(task.program, " ");
      }
    }


    ssize_t bytes_read = 0;
    char fifo_name[50];
    sprintf(fifo_name, CLIENT "_%d", task.pid);

    // Create client FIFO

    if (mkfifo(fifo_name, 0644) < 0) {
      perror("Error creating FIFO");
      exit(1);
    }

    // Open server FIFO

    fd_servidor = open(SERVER, O_WRONLY);

    if (fd_servidor < 0) {
      perror("Error opening server FIFO for writing");
      exit(1);
    }

    // Write task to server
    write(fd_servidor, &task, sizeof(task));

    // Open client FIFO for reading
    sprintf(fifo_name, CLIENT "_%d", getpid());

    if ((fd_cliente = open(fifo_name, O_RDONLY)) < 0) {
      perror("Error opening FIFO");
      exit(1);
    }

    // Read from client FIFO and write to stdout
    char buffer[1024];
    while ((bytes_read = read(fd_cliente, buffer, sizeof(buffer))) > 0) {
      write(STDOUT_FILENO, buffer, bytes_read);
    }
    close(fd_cliente);
    _exit(0);

  }
  else {
    wait(NULL);
  }

  close(fd_servidor);
  return 0;
}
