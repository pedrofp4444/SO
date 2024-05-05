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

int main(int argc, char* argv[]) {
  // Verifies if the number of arguments is correct
  if (argc < 4) {
    char* usage = "Usage: <output_folder> <parallel-tasks> <sched-policy>\n";
    write(1, usage, strlen(usage));
    return 1;
  }

  // Inicializates the variables
  int ids = 1;
  char* output_folder = argv[1];
  int parallel_tasks = strtol(argv[2], NULL, 10);
  char* scheduling_algorithm = argv[3];

  int aux_tasks = 0;

  // Verifies if the output folder exists, if not, creates it
  struct stat st = { 0 };
  if (stat(output_folder, &st) == -1) {
    mkdir(output_folder, 0700);
  }

  // Prints the configuration
  // printf("=== (Orchestrator started) ===\n");
  // printf("[USING] output_folder: %s\n", output_folder);
  // printf("[USING] parallel_tasks: %d\n", parallel_tasks);
  // printf("[USING] scheduling_algorithm: %s\n", scheduling_algorithm);

  // Inicializes the queue and the fifo for the comunications between the orchestrator and the clients
  Queue* queue = createQueue();
  createFIFO(ORCHESTRATOR);
  int orchestrator_fifo = openFIFO(ORCHESTRATOR, O_RDONLY);
  Status* task_status = createStatus();

  // Creates a pipe for the communication between the reader process and the orchestrator
  int fds_read_orchestrator[2];
  pipe(fds_read_orchestrator);

  // Creates the reader process
  if (fork() == 0) {
    close(fds_read_orchestrator[0]);

    while (1) {
      // Declares the task variable
      Task task;

      // Reads the task from the orchestrator fifo, receiving the tasks from the clients
      if (read(orchestrator_fifo, &task, sizeof(task)) > 0) {
        if (task.phase == NONE) {
          // Gets the start time of the task, which corresponds to the time when the reader process receives the task
          gettimeofday(&task.start_time, NULL);

          // Assigns the id to the task, making it incremental and unique

          if (task.type != STATUS) {
            // Opens the fifo created by the client, to send the task id back to the client
            task.id = ids++;
          }
          char fifo_name[50];
          sprintf(fifo_name, CLIENT "_%d", task.pid);
          int fd_client = open(fifo_name, O_WRONLY);

          // Writes the task id to the client
          write(fd_client, &task, sizeof(task));
          // write(fd_client, &task_status, sizeof(task_status));

          // Closes the writing file descriptor of the client fifo
          close(fd_client);
        }

        // Writes the task to the orchestrator pipe, to be read and managed by the orchestrator process
        write(fds_read_orchestrator[1], &task, sizeof(task));
      }
    }

    // Closes the orchestrator fifo to stop receiving tasks
    close(orchestrator_fifo);

    // Closes the writing file descriptor of the reader-orchestrator pipe to stop sending tasks to be executed
    close(fds_read_orchestrator[1]);

    // Exits the reader process
    _exit(0);
  }
  else {
    // The main process, which is the orchestrator process, will run

    // Close the write end of the pipe, once the orchestrator will only read from it
    close(fds_read_orchestrator[1]);

    while (1) {
      // Declares the task variable
      Task task;

      // Reads the task from the reader-orchestrator pipe
      if (read(fds_read_orchestrator[0], &task, sizeof(task)) > 0) {
        // Inserts the task in the queue

        if (task.type == STATUS) {
          if (fork() == 0) {
            char fifo_name[50];
            sprintf(fifo_name, CLIENT "_%d", task.pid);
            int fd_client = open(fifo_name, O_WRONLY);

            // print_status(task_status);

            write(fd_client, task_status, sizeof(Status));

            // Closes the writing file descriptor of the client fifo
            close(fd_client);
            _exit(0);
          }

        }
        else if (task.phase != NONE) {
          if (task.phase == COMPLETED) {
            aux_tasks--;
          }
          else if (task.phase == EXECUTING) {
            aux_tasks++;
          }
          updateStatus(task_status, task);
          print_status(task_status);

        }
        else {
          enqueueStatus(task_status, task);
          // print_status(task_status);
          enqueue(queue, task);
          // print_queue(queue);
        }
      }

      // Creates a pipe for the communication between the orchestrator and the solvers child processes to get information for the logs
      // int pipe_logs[2];
      // pipe(pipe_logs);

      // Verifies if the queue has more than 2 tasks before executing them [THIS IS JUST DEBUG AND NEEDS TO BE REMOVED, ONLY THE IF STATEMENT]
      if (queue->size > 2) {
        // Variable to keep track of the number of tasks that are being executed

        // While there are tasks to be executed and the number of parallel tasks is not reached, proceed to manage the tasks and execute them

        if (!isEmpty(queue)) {

          // print_queue(queue);

          printf("aux_tasks: %d\n", aux_tasks);
          if (aux_tasks < parallel_tasks) {
            Task task_aux;
            if (strcmp(scheduling_algorithm, "sjf") == 0) {
              task_aux = dequeue_with_priority(queue);
            }
            else {
              task_aux = dequeue(queue);
            }


            // -----------------------------------------------------------------------
            int main_fifo = openFIFO(ORCHESTRATOR, O_WRONLY);
            task_aux.phase = EXECUTING;
            write(main_fifo, &task_aux, sizeof(task_aux));
            close(main_fifo);
            // -----------------------------------------------------------------------


            if (fork() == 0) {
              if (fork() == 0) {
                // close(pipe_logs[0]);

                char output_path[PATH_MAX];
                snprintf(
                  output_path, sizeof(output_path), "%s/task_%d.output",
                  output_folder, task_aux.id
                );

                char* program = strdup(task_aux.program);

                int number_of_commands = count_commands(program);

                char* task_commands[number_of_commands];
                split_commands(program, task_commands, number_of_commands);

                execute_task(number_of_commands, task_commands, output_path);

                free(program);

                // struct timeval end_time, duration;

                // gettimeofday(&end_time, NULL);

                // timersub(&end_time, &task.start_time, &duration);

                // write(pipe_logs[1], &duration, sizeof(duration));

                // close(pipe_logs[1]);

                _exit(task_aux.id);
              }
              else {
                int status;
                wait(&status);
                if (WIFEXITED(status)) {

                  // -----------------------------------------------------------------------
                  int main_fifo = openFIFO(ORCHESTRATOR, O_WRONLY);

                  Task task_finished = findTask(task_status, WEXITSTATUS(status));
                  task_finished.phase = COMPLETED;

                  struct timeval end_time, duration;

                  gettimeofday(&end_time, NULL);

                  timersub(&end_time, &task_finished.start_time, &duration);

                  // task_finished.start_time = duration;


                  write(main_fifo, &task_finished, sizeof(task_aux));
                  close(main_fifo);
                  // -----------------------------------------------------------------------

                  int log_fd = open("log", O_WRONLY | O_CREAT | O_APPEND, 0644);

                  if (log_fd != -1) {
                    char log_message[256];

                    // Formats the message to be written in the lprint_queue(queue);ogs file
                    int message_length = snprintf(
                      log_message, sizeof(log_message),
                      "Task ID: %d, Duration: %ld.%06ld seconds\n",
                      WEXITSTATUS(status), duration.tv_sec, duration.tv_usec
                    );

                    // Writes the message to the logs file
                    write(log_fd, log_message, message_length);

                    // Closes the logs file
                    close(log_fd);
                  }
                }

                _exit(0);
              }
              // The orchestrator main process decrements the number of tasks being executed

              _exit(0);
            }

          }

        }
      }

      // Closes the reading file descriptor of the reader-orchestrator pipe
    }
    close(fds_read_orchestrator[0]);

    // Closes the orchestrator fifo
    close(orchestrator_fifo);

    // Frees the queue
    freeQueue(queue);
  }
  return 0;
}
