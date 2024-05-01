#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_COMMANDS 10
#define PATH_MAX 4096

int createFiFO(char* name) {
  int result = mkfifo(name, 0644);
  if (result < 0) {
    perror("Error creating FIFO");
    return 0;
  }
  return 1;
}

int openFiFO(char* name, int mode) {
  int result = open(name, mode);
  if (result < 0) {
    perror("Error opening FIFO");
    return 0;
  }
  return result;
}

void writeFiFO(int fd, void* data, size_t size) {
  write(fd, data, size);
}

void readFiFO(int fd, void* data, size_t size) {
  read(fd, data, size);
}

void closeFiFO(int fd) {
  close(fd);
}

void parseInstructions(char* program, char* instructions[]) {
  char* token;
  int i = 0;

  // Use a copy of the program string since strsep modifies the original string
  char* program_copy = strdup(program);

  if (program_copy == NULL) {
    perror("Error duplicating string");
    exit(1);
  }

  // Tokenize the program string
  while ((token = strsep(&program_copy, " ")) != NULL) {
    instructions[i] = token;
    i++;
  }

  // Null-terminate the instructions array
  instructions[i] = NULL;

  // Free the duplicated string
  free(program_copy);
}

void executeTask(char* instructions[]) {
  execvp(instructions[0], instructions);
  perror("execvp");
  exit(1);
}

void redirectStdout(int pipefd[2]) {
  if (pipe(pipefd) == -1) {
    perror("pipe");
    exit(1);
  }

  if (fork() == 0) {
    close(pipefd[0]);
    dup2(pipefd[1], STDOUT_FILENO);
  }
}

void redirectStdin(int pipefd[2]) {
  if (pipe(pipefd) == -1) {
    perror("pipe");
    exit(1);
  }

  if (fork() == 0) {
    close(pipefd[1]);
    dup2(pipefd[0], STDIN_FILENO);
  }
}

#define MAX_COMMANDS 10
#define PATH_MAX 4096

int exec_command(char* arg) {
  char* exec_args[MAX_COMMANDS];
  char* string;
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
  int in_fd = 0;  // Início da pipeline: stdin
  int out_fd;     // Descritor de arquivo para o arquivo de saída

  for (i = 0; i < number_of_commands; i++) {
    int fd[2];
    pipe(fd);

    if (fork() == 0) {
      dup2(in_fd, STDIN_FILENO);  // Define o stdin do processo filho
      if (i < number_of_commands - 1) {
        dup2(fd[1], STDOUT_FILENO);  // Define o stdout do processo filho
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
      close(fd[0]);  // Fecha o lado de leitura do pipe não utilizado pelo filho

      exec_command(commands[i]);
      _exit(EXIT_FAILURE);  // exec_command só retorna se houver erro
    } else {
      wait(NULL);    // Espera o processo filho terminar
      close(fd[1]);  // Fecha o lado de escrita do pipe no processo pai
      in_fd = fd[0];  // O próximo comando usará a saída deste pipe como entrada
    }
  }

  return 0;
}

// Função para contar o número de comandos baseado no delimitador '|'
int count_commands(char* program) {
  int count = 1;
  const char* tmp = program;
  while ((tmp = strchr(tmp, '|')) != NULL) {
    count++;
    tmp++;
  }
  return count;
}

// Função para dividir a string do programa em comandos individuais
void split_commands(
    char* program, char** task_commands, int number_of_commands
) {
  const char delim[2] = "|";
  char* token;
  token = strtok(program, delim);
  int i = 0;
  while (token != NULL && i < number_of_commands) {
    task_commands[i] = token;
    token = strtok(NULL, delim);
    i++;
  }
}
