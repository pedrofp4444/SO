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

#define MAX_COMMANDS 10

int exec_command(char* arg) {
    char *exec_args[MAX_COMMANDS];
    char *string;
    int i = 0;

    char* command = strdup(arg);
    string = strtok(command, " ");
    
    while (string != NULL) {
        exec_args[i] = string;
        string = strtok(NULL, " ");
        i++;
    }
    exec_args[i] = NULL;
    
    if (execvp(exec_args[0], exec_args) == -1) {
      perror("execvp error");
      _exit(EXIT_FAILURE);
    }

    return 0;
}

void execute_task(Task task, char* output_folder) {
    char* instructions[MAX_COMMANDS];
    parseInstructions(task.program, instructions);

    int number_of_commands = 0;
    while (instructions[number_of_commands] != NULL) {
        number_of_commands++;
    }

    int in_fd = 0; // Início da pipeline: stdin
    int i;
    char filename[256];

  printf("FOLDER: %s\n", output_folder);
    // Cria o nome do arquivo com base no ID da tarefa e no diretório de saída
    snprintf(filename, sizeof(filename), "%s/task_%d_output.txt", output_folder, task.id);

    // Abre o arquivo para escrita
    int file_fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (file_fd < 0) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < number_of_commands; i++) {
        int fd[2];
        if (pipe(fd) == -1) {
            perror("pipe error");
            exit(EXIT_FAILURE);
        }

        pid_t pid = fork();
        if (pid == -1) {
            perror("fork error");
            exit(EXIT_FAILURE);
        }

        if (pid == 0) {
            if (dup2(in_fd, STDIN_FILENO) == -1) {
                perror("dup2 stdin error");
                _exit(EXIT_FAILURE);
            }
            if (i < number_of_commands - 1) {
                if (dup2(fd[1], STDOUT_FILENO) == -1) {
                    perror("dup2 stdout error");
                    _exit(EXIT_FAILURE);
                }
            } else {
                if (dup2(file_fd, STDOUT_FILENO) == -1) {
                    perror("dup2 file_fd error");
                    _exit(EXIT_FAILURE);
                }
            }
            close(fd[0]);
            close(fd[1]); // Importante fechar o descritor de escrita no processo filho

            exec_command(instructions[i]);
        } else {
            wait(NULL); // Espera o processo filho terminar
            close(fd[1]); // Fecha o lado de escrita do pipe no processo pai
            if (i == number_of_commands - 1) {
                close(fd[0]); // Se for o último comando, fecha o lado de leitura do pipe
            } else {
                in_fd = fd[0]; // O próximo comando usará a saída deste pipe como entrada
            }
        }
    }

    // Fecha o arquivo
    close(file_fd);
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        printf("Usage: <output_folder> <parallel-tasks> <sched-policy>\\n");
        return 1;
    }

    int ids = 1;
    char* output_folder = argv[1];
    int parallel_tasks = strtol(argv[2], NULL, 10);
    char* scheduling_algorithm = argv[3];

    // Verifica se a pasta de saída existe, se não, tenta criá-lo
    struct stat st = {0};
    if (stat(output_folder, &st) == -1) {
        mkdir(output_folder, 0700);
    }

    printf("output_folder: %s\\n", output_folder);
    printf("parallel_tasks: %d\\n", parallel_tasks);
    printf("scheduling_algorithm: %s\\n", scheduling_algorithm);

    // Inicialização da fila e outras estruturas necessárias
    Queue* queue = createQueue();
    createFiFO(SERVER);
    int server_fd = openFiFO(SERVER, O_RDONLY);

    while (1) {
        Task task;
        if (read(server_fd, &task, sizeof(task)) > 0) {
            printf("Received task_%d: %s\\n", task.pid, task.program);

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
                Task task_aux;
                if (strcmp(scheduling_algorithm, "sjf") == 0) {
                    task_aux = dequeue_Priority(queue);
                } else {
                    task_aux = dequeue(queue);
                }

                if (fork() == 0) {
                    execute_task(task_aux, output_folder);
                    _exit(0);
                } else {
                    aux_tasks++;
                }
            }

            while (aux_tasks > 0) {
                wait(NULL);
                aux_tasks--;
            }
        }
    }

    close(server_fd);
    freeQueue(queue);

  return 0;
}
