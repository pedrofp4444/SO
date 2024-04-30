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
#define PATH_MAX 4096

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
    
    return execvp(exec_args[0], exec_args);
}

int execute_task(int number_of_commands, char** commands, char* output_file) {
    int i;
    int in_fd = 0; // Início da pipeline: stdin
    int out_fd; // Descritor de arquivo para o arquivo de saída

    for (i = 0; i < number_of_commands; i++) {
        int fd[2];
        pipe(fd);

        if (fork() == 0) {
            dup2(in_fd, STDIN_FILENO); // Define o stdin do processo filho
            if (i < number_of_commands - 1) {
                dup2(fd[1], STDOUT_FILENO); // Define o stdout do processo filho
            } else {
                // Para o último comando, redireciona a saída para o arquivo especificado
                out_fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (out_fd < 0) {
                    perror("open");
                    exit(EXIT_FAILURE);
                }
                dup2(out_fd, STDOUT_FILENO);
                close(out_fd);
            }
            close(fd[0]); // Fecha o lado de leitura do pipe não utilizado pelo filho

            exec_command(commands[i]);
            _exit(EXIT_FAILURE); // exec_command só retorna se houver erro
        } else {
            wait(NULL); // Espera o processo filho terminar
            close(fd[1]); // Fecha o lado de escrita do pipe no processo pai
            in_fd = fd[0]; // O próximo comando usará a saída deste pipe como entrada
        }
    }

    return 0;
}

// Função para contar o número de comandos baseado no delimitador '|'
int count_commands(char* program) {
    int count = 1;
    const char* tmp = program;
    while((tmp = strchr(tmp, '|')) != NULL) {
        count++;
        tmp++;
    }
    return count;
}

// Função para dividir a string do programa em comandos individuais
void split_commands(char* program, char** task_commands, int number_of_commands) {
    const char delim[2] = "|";
    char *token;
    token = strtok(program, delim);
    int i = 0;
    while(token != NULL && i < number_of_commands) {
        task_commands[i] = token;
        token = strtok(NULL, delim);
        i++;
    }
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

    // Verifica se a pasta de saída existe, se não, tenta criá-la
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
    
    // Cria um pipe para comunicação entre processos
    int fd[2];
    pipe(fd);

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

                if (fork() == 0) {
                    
                    char output_path[PATH_MAX];
                    snprintf(output_path, sizeof(output_path), "%s/task_%d.output", output_folder, task_aux.id);
                    char* program = strdup(task_aux.program); // Duplica a string para evitar modificar o original
                    int number_of_commands = count_commands(program);
                    char* task_commands[number_of_commands]; // Cria um array de strings para os comandos

                    split_commands(program, task_commands, number_of_commands);

                    // Executa a tarefa
                    execute_task(number_of_commands, task_commands, output_path);

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
