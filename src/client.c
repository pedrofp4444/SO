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
  Task task;
  pid_t pid = getpid();

  char fifo_name[50];
  sprintf(fifo_name, CLIENT "_%d", pid);
  createFiFO(fifo_name);

  if (strcmp(argv[0], "status") == 0) {
    task.type = STATUS;
  } else if (argc < 5) {
    printf(
        "Usage: %s <flag [-u // -p]> <duration> <program & args>\n", argv[0]
    );
    exit(1);

  } else {
    task.type = TASK;
  }
  task.id = -1;
  task.pid = pid;
  task.duration = strtol(argv[3], NULL, 10);
  strcpy(task.program, argv[4]);

  int fd_server = openFiFO(SERVER, O_WRONLY);
  write(fd_server, &task, sizeof(task));

  int bytes_read = 0;

  int fd_client = openFiFO(fifo_name, O_RDONLY);
  if (read(fd_client, &task, sizeof(task)) > 0) {
    printf("Task id: %d\n", task.id);
  }

  close(fd_server);
  return 0;
}

/*

velhos: +13 7 / 8
novos :


 int fd_servidor, fd_cliente;
  Task task;
  task.duration =
      strtol(argv[3], NULL, 10);  // Convert duration argument to integer
  task.pid = getpid();
  strcpy(task.program, argv[4]);
  printf("task.id: %d\n", task.pid);

  ssize_t bytes_read = 0;
  char fifo_name[50];
  sprintf(fifo_name, CLIENT "_%d", task.pid);

  // Create client FIFO
  createFiFO(fifo_name);

  // Open server FIFO
  fd_servidor = openFiFO(SERVER, O_WRONLY);

  // Write task to server
  writeFiFO(fd_servidor, &task, sizeof(task));

  close(fd_cliente);
  return 0;
*/
