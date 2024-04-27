#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "taskmanager.h"
#include "utils.h"

/*
  server output 10 sjf
*/

int main(int argc, char* argv[]) {
  if (argc < 4) {
    printf("Usage: <output_folder> <parallel-tasks> <sched-policy>");
    return 1;
  }
  int ids = 1;
  char* output_folder = argv[1];
  int parallel_tasks = strtol(argv[2], NULL, 10);
  char* scheduling_algorithm = argv[3];

  printf("output_folder: %s\n", output_folder);
  printf("parallel_tasks: %d\n", parallel_tasks);
  printf("scheduling_algorithm: %s\n", scheduling_algorithm);

  Queue* queue = createQueue();
  createFiFO(SERVER);
  int server_fd = openFiFO(SERVER, O_RDONLY);

  while (1) {
    Task task;
    if (read(server_fd, &task, sizeof(task)) > 0) {
      printf("Received task_%d: %s\n", task.pid, task.program);

      task.id = ids++;
      enqueue(queue, task);
      print_queue(queue);

      char fifo_name[50];
      sprintf(fifo_name, CLIENT "_%d", task.pid);
      int fd_client = open(fifo_name, O_WRONLY);
      write(fd_client, &task, sizeof(task));
      close(fd_client);
    }
    if (queue->size > 3) {
      int aux_tasks = 0;
      while (aux_tasks < parallel_tasks && !isEmpty(queue)) {
        printf("aux_tasks 1º: %d\n", aux_tasks);
        Task task_aux;
        if (strcmp(scheduling_algorithm, "sjf") == 0) {
          printf("================sjf\n");
          task_aux = dequeue_Priority(queue);
          printf("Dequeued task_%d: %s\n", task_aux.pid, task_aux.program);
          print_queue(queue);
          printf("================sjf\n");
        }
        else {
          printf("----------------------fifo\n");
          task_aux = dequeue(queue);
          printf("Dequeued task_%d: %s\n", task_aux.pid, task_aux.program);
          print_queue(queue);
          printf("------------------------fifo\n");
        }
        aux_tasks++;
        if (fork() == 0) {
          char* instructions[100];
          parseInstructions(task_aux.program, instructions);
          printf(
            "-----FORK------\n+++++++task_aux.program: %s\n", task_aux.program
          );
          _exit(0);
        }
        printf("aux_tasks 2º: %d\n", aux_tasks);
      }

      if (aux_tasks >0) {
        aux_tasks--;
        wait(NULL);
      }
    }
  }

  return 0;
}

/*
 int server_fd;
  int completed_tasks = 0;
  struct timeval start, end;
  long elapsed_time;
  int parallel_tasks = strtol(argv[2], NULL, 10);
  int aux_parallel_taks = 0;
  char* scheduling_algorithm = argv[3];
  // char* path_of_completed_tasks = argv[1];

  completed_tasks = open(argv[1], O_CREAT | O_RDWR, 0644);
  // Create server FIFO
  createFiFO(SERVER);
  Queue* queue = createQueue();
  printf("queue.size: %d\n", queue->end);

  // Open server FIFO for reading CLIENT
  server_fd = openFiFO(SERVER, O_RDONLY);
  while (1) {
    Task task;
    if (read(server_fd, &task, sizeof(task)) > 0) {
      enqueue(queue, task);
      printf("Received task_%d: %s\n", task.pid, task.program);
      print_queue(queue);
    }

    int fd_pipe[2];
    if(pipe(fd_pipe)==-1){
      perror("Error creating pipe");
      return 1;
    }

    if (fork()==0){

    }
3

  }
*/