#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "utils.h"

int main(int argc, char* argv[]) {
  if (argc < 5) {
    printf("Usage: %s <flag [-u // -p]> <duration> <program & args>\n", argv[0]);
    exit(1);
  }

  int fd_servidor, fd_cliente;
  Task task;
  task.duration =
      strtol(argv[3], NULL, 10);  // Convert duration argument to integer
  task.pid = getpid();
  strcpy(task.program, argv[4]);

  printf("Received task_%d: %s\n", task.pid, task.program);

  ssize_t bytes_read = 0;
  char fifo_name[50];
  sprintf(fifo_name, CLIENT "_%d", task.pid);

  // Create client FIFO
  createFiFO(fifo_name);

  // Open server FIFO
  fd_servidor = openFiFO(SERVER, O_WRONLY);

  // Write task to server
  writeFiFO(fd_servidor, &task, sizeof(task));

  // Open client FIFO for reading
  sprintf(fifo_name, CLIENT "_%d", getpid());
  fd_cliente = openFiFO(fifo_name, O_RDONLY);

  // Read from client FIFO and write to stdout
  char buffer[1024];
  while ((bytes_read = read(fd_cliente, buffer, sizeof(buffer))) > 0) {
    write(STDOUT_FILENO, buffer, bytes_read);
  }

  close(fd_cliente);
  return 0;
}
