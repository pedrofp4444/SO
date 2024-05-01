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
    printf("Usage: <output_folder> <parallel-tasks> <sched-policy>\n");
    return 1;
  }

  int ids = 1;
  char* output_folder = argv[1];
  int parallel_tasks = strtol(argv[2], NULL, 10);
  char* scheduling_algorithm = argv[3];

  // Verifica se a pasta de saída existe, se não, tenta criá-la
  struct stat st = {0};
  if (stat(output_folder, &st) == -1) {
    mkdir(output_folder, 0700);
  }

  printf("output_folder: %s\n", output_folder);
  printf("parallel_tasks: %d\n", parallel_tasks);
  printf("scheduling_algorithm: %s\n", scheduling_algorithm);

  // Inicialização da fila e outras estruturas necessárias
  Queue* queue = createQueue();
  createFiFO(SERVER);
  int server_fd = openFiFO(SERVER, O_RDONLY);

  // Cria um pipe para comunicação entre processos
  int fd_read_server[2];
  pipe(fd_read_server);

  if (fork() == 0) {
    close(fd_read_server[0]);

    while (1) {
        Task task;
      if (read(server_fd, &task, sizeof(task)) > 0) {
        gettimeofday(&task.start_time, NULL);

        printf("Received task_%d: %s\n", task.pid, task.program);

        task.id = ids++;

        char fifo_name[50];
        sprintf(fifo_name, CLIENT "_%d", task.pid);
        int fd_client = open(fifo_name, O_WRONLY);
        write(fd_client, &task, sizeof(task));
        close(fd_client);

        write(fd_read_server[1], &task, sizeof(task));
      }
    }
    close(server_fd);
    close(fd_read_server[1]);
    _exit(0);
  } else {
    close(fd_read_server[1]);

    while (1) {

        Task task;
        int bytes_read = 0;
        if(bytes_read = read(fd_read_server[0], &task, sizeof(task)) > 0) {
            printf("Task program no PAI: %s\n", task.program);
            enqueue(queue, task);
        }

        int pipe_logs[2]; // Já tenho aqui a declaração do pipe
        pipe(pipe_logs);

      if (queue->size > 2) {
        int aux_tasks = 0;
        while (aux_tasks < parallel_tasks && !isEmpty(queue)) {
          Task task_aux;
          if (strcmp(scheduling_algorithm, "sjf") == 0) {
            task_aux = dequeue_Priority(queue);
          } else {
            task_aux = dequeue(queue);
          }

          printf("Task: %s\n", task_aux.program);

          if (fork() == 0) { // Este é o filho
            close(pipe_logs[0]); // Fecha a extremidade de leitura do pipe no filho
            printf("ENTROU AQUI\n");
            char output_path[PATH_MAX];
            snprintf(
                output_path, sizeof(output_path), "%s/task_%d.output",
                output_folder, task_aux.id
            );
            char* program = strdup(task_aux.program
            );  // Duplica a string para evitar modificar o original
            int number_of_commands = count_commands(program);
            char* task_commands
                [number_of_commands];  // Cria um array de strings para os comandos

            split_commands(program, task_commands, number_of_commands);

            printf("Output path: %s\n", output_path);

            // Executa a tarefa
            execute_task(number_of_commands, task_commands, output_path);

            struct timeval end_time, duration;
            gettimeofday(&end_time, NULL);
            timersub(&end_time, &task.start_time, &duration);
            
            // Escreve a duração no pipe
            write(pipe_logs[1], &duration, sizeof(duration));
            close(pipe_logs[1]); // Fecha a extremidade de escrita do pipe no filho

            _exit(task_aux.id);
          } else {
            aux_tasks++;
          }
        }

        close(pipe_logs[1]); // Fecha a extremidade de escrita do pipe no pai

        while (aux_tasks > 0) {
            int status;
            wait(&status); // Espera por um processo filho terminar

            if (WIFEXITED(status)) { // Se o filho terminou normalmente
                struct timeval duration;
                read(pipe_logs[0], &duration, sizeof(duration)); // Lê a duração do pipe

                // Abre o ficheiro de logs para escrita
                int log_fd = open("logs", O_WRONLY | O_CREAT | O_APPEND, 0644);
                if (log_fd != -1) {
                    char log_message[256];
                    int message_length = snprintf(log_message, sizeof(log_message),
                                                  "Task ID: %d, Duration: %ld.%06ld seconds\n",
                                                  WEXITSTATUS(status), duration.tv_sec, duration.tv_usec);

                    write(log_fd, log_message, message_length); // Escreve no arquivo de logs
                    close(log_fd); // Fecha o arquivo de logs
                }
            }
            aux_tasks--;
        }
        close(pipe_logs[0]); // Fecha a extremidade de leitura do pipe no pai
      }
    }
    close(fd_read_server[0]);
  }

  close(server_fd);
  freeQueue(queue);

  return 0;
}
