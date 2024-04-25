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

  int pipefd1[2];  // Parent writes, child reads
  int pipefd2[2];  // Parent reads, child writes

  if (pipe(pipefd1) == -1 || pipe(pipefd2) == -1) {
    perror("pipe");
    return 1;
  }

  // Fork the process for enqueuing tasks
  if (fork() == 0) {
    close(pipefd1[1]);  // Close the write end of pipefd1 in the child process
    close(pipefd2[0]);  // Close the read end of pipefd2 in the child process

    Task task;
    Queue* queue = createQueue();
    while (1) {
      while (read(server_fd, &task, sizeof(task)) > 0) {
        enqueue(queue, task);
        printf("Received task_%d: %s\n", task.pid, task.program);
        print_queue(queue);

        // Write the queue to pipefd2
        write(pipefd2[1], queue, sizeof(Queue));

        // Read the queue from pipefd1
        read(pipefd1[0], queue, sizeof(Queue));
      }
    }
    // Clean up resources
    close(pipefd1[0]);
    close(pipefd2[1]);
    close(server_fd);
    exit(0);
  }

  // Fork the process for dequeuing and executing tasks
  if (fork() == 0) {
    close(pipefd1[0]);  // Close the read end of pipefd1 in the child process
    close(pipefd2[1]);  // Close the write end of pipefd2 in the child process

    while (1) {
      Queue* received_queue = malloc(sizeof(Queue));

      // Read the queue from pipefd2
      read(pipefd2[0], received_queue, sizeof(Queue));
      printf("Received queue from pipefd2:\n");
      print_queue(received_queue);

      while (1) {
        Task task_aux;

        if (!isEmpty(received_queue) && aux_parallel_taks < parallel_tasks) {
          printf("dentro do if empty\n\n");

          if (strcmp(scheduling_algorithm, "sjf") == 0) {
            task_aux = dequeue_Priority(received_queue);
          } else {
            task_aux = dequeue(received_queue);
          }

          if (fork() == 0) {
            // Child process
            char* instructions[50];
            parseInstructions(task_aux.program, instructions);

            // Open client FIFO for writing
            char fifo_name[50];
            sprintf(fifo_name, CLIENT "_%d", task_aux.pid);
            int fd_client = openFiFO(fifo_name, O_WRONLY);

            // Redirect stdout to client FIFO
            dup2(fd_client, STDOUT_FILENO);

            // Execute the task
            executeTask(instructions);

            // Close client FIFO
            close(fd_client);

            exit(0);  // Exit child process

          } else {
            // Parent process
            wait(NULL);
            aux_parallel_taks--;

            // Open client FIFO for reading
            char fifo_name[50];
            sprintf(fifo_name, CLIENT "_%d", task_aux.pid);
            int fd_client = openFiFO(fifo_name, O_RDONLY);

            // Read from client FIFO and write back to the client
            char buffer[1024];
            ssize_t nbytes = 0;

            while ((nbytes = read(fd_client, buffer, sizeof(buffer))) > 0) {
              write(fd_client, buffer, nbytes);
            }

            // Close client FIFO
            close(fd_client);

            // Write completion message to the shared FIFO (completed_tasks)
            char task_done[100];
            sprintf(
                task_done, "Task %d executed in %d milliseconds\n",
                task_aux.pid, task_aux.duration
            );

            writeFiFO(completed_tasks, task_done, strlen(task_done));
            close(completed_tasks);
          }
        }
      }
      write(pipefd1[1], received_queue, sizeof(Queue));

      free(received_queue);
    }

    // Clean up resources
    close(pipefd1[1]);
    close(pipefd2[0]);
    exit(0);
  }

  wait(NULL);
  wait(NULL);
  return 0;
}

/* 3 tentaiva
 Task task;

  int bytes_read = 0;
  while (1) {
    while ((bytes_read = read(fd, &task, sizeof(task))) > 0) {

      gettimeofday(&start, NULL);
      printf("Received task_%d: %s\n", task.pid, task.program);
      enqueue(queue, task);
      printf("queue.size: %d\n", queue->end);
      printf("Queue: \n");

      print_queue(queue);


      if (aux_parallel_taks < parallel_tasks) {
        aux_parallel_taks++;

        int pipefd[2];
        int fd_client;

        if (fork() == 0) {


          Task task_aux;
          if (strcmp(scheduling_algorithm, "sjf") == 0) {
            task_aux = dequeue_Priority(queue);
          }
          else {
            task_aux = dequeue(queue);
          }

          char fifo_name[50];
          sprintf(fifo_name, CLIENT "_%d", task_aux.pid);
          fd_client = openFiFO(fifo_name, O_WRONLY);
          printf("fd_client: %d\n", fd_client);

          printf("fifo_name: %s\n", fifo_name);

          // Redirect stdout to a pipe
          close(pipefd[0]);
          dup2(pipefd[1], STDOUT_FILENO);
          close(pipefd[1]);

          char* instructions[50];
          parseInstructions(task_aux.program, instructions);
          executeTask(instructions);

          gettimeofday(&end, NULL);
          elapsed_time = (end.tv_sec - start.tv_sec) * 1000 +
            (end.tv_usec - start.tv_usec) / 1000;



          exit(0);
        }
        else {
          wait(NULL);
          close(pipefd[1]);
          char buffer[1024];
          ssize_t nbytes = 0;
          while ((nbytes = read(pipefd[0], buffer, sizeof(buffer))) > 0) {
            // Write the output of the task back to the client
            write(fd_client, buffer, nbytes);
          }
          close(pipefd[0]);
          closeFiFO(fd_client);

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



          write(STDOUT_FILENO, "Task completed\n", 15);


        }
      }

    }

  }
*/

/* tentativa 2

      printf("Received task_%d: %s\n", task.pid, task.program);
      enqueue(queue, task);
      printf("queue.size: %d\n", queue->end);
      printf("Queue: \n");
      print_queue(queue);

      if (aux_parallel_taks < parallel_tasks) {
        aux_parallel_taks++;

        int fd_client;
        int pipefd[2];

        if (fork() == 0) { // Child process
          // Dequeue the task to be executed
          if (strcmp(scheduling_algorithm, "sjf") == 0) {
            task_aux = dequeue_Priority(queue);
          }
          else {
            task_aux = dequeue(queue);
          }

          // Parse task instructions and execute
          char* instructions[100];
          parseInstructions(task_aux.program, instructions, MAX_TASKS);
          executeTask(task_aux.program, instructions);

          // End timer
          gettimeofday(&end, NULL);

          // Open client FIFO for writing response
          char fifo_name[50];
          sprintf(fifo_name, CLIENT "_%d", task_aux.pid);
          int fd_client = openFiFO(fifo_name, O_WRONLY);

          // Send completion message to client
          elapsed_time = (end.tv_sec - start.tv_sec) * 1000 +
            (end.tv_usec - start.tv_usec) / 1000;

          char completed_tasks_msg[100];
          sprintf(completed_tasks_msg, "Task %d executed in %ld milliseconds\n",
            task_aux.pid, elapsed_time);
          writeFiFO(completed_tasks, completed_tasks_msg, strlen(completed_tasks_msg));

          // Close client FIFO
          closeFiFO(fd_client);

          // Exit child process
          _exit(0);
        }
        else { // Parent process
          // Wait for child process to finish
          wait(NULL);

          printf("Task completed\n");

          closeFiFO(fd_client);
          // Decrease the count of parallel tasks being executed
          aux_parallel_taks--;
        }
      }
    }
*/

/* tentativa 1
        while (1) {
          Task task;
          int bytes_read = 0;
          while ((bytes_read = read(fd, &task, sizeof(task))) > 0) {
            // Start timer

            if (fork() == 0) {
              // Start timer
              gettimeofday(&start, NULL);
              printf("Received task_%d: %s\n", task.pid, task.program);

              enqueue(&queue, task);
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
                if (strcmp(scheduling_algorithm, "sjf") == 0) {
                  Task task_aux = dequeue_Priority(&queue);
                }
                else {
                  Task task_aux = dequeue(&queue);
                }

                char* instructions[100];
                parseInstructions(task.program, instructions, MAX_TASKS);
                executeTask(task.program, instructions);

                // End timer
                gettimeofday(&end, NULL);
              }
              else {
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
        */