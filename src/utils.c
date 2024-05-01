#include "utils.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int createFIFO(char* name) {
  // Creates a FIFO with the given name
  int result = mkfifo(name, 0644);
  if (result < 0) {
    perror("Error creating FIFO");
    return 0;
  }
  return 1;
}

int openFIFO(char* name, int mode) {
  // Opens a FIFO with the given name
  int result = open(name, mode);
  if (result < 0) {
    perror("Error opening FIFO");
    return 0;
  }
  return result;
}

int exec_command(char* arg, int number_of_commands) {
  // Initializes the array of arguments to be passed to execvp
  char* exec_args[number_of_commands + 1];

  // String where to split the command string into an array of arguments
  char* string;

  // Index of the current argument
  int i = 0;

  // Duplicates the command string to avoid modifying the original string
  char* command = strdup(arg);
  string = strtok(command, " ");

  // Splits the command string into an array of arguments
  while (string != NULL) {
    exec_args[i] = string;
    string = strtok(NULL, " ");
    i++;
  }

  // Sets the last element of the array to NULL
  exec_args[i] = NULL;

  // Executes the command
  return execvp(exec_args[0], exec_args);
}

int execute_task(int number_of_commands, char** commands, char* output_file) {
  // Index of the current command
  int i;

  // Descriptor for the beginning of the pipeline
  int in_fd = 0;

  // Descriptor for the output file
  int out_fd;

  // Executes each command in the task
  for (i = 0; i < number_of_commands; i++) {
    // Creates a pipe to stablish the communication between the processes
    int fd[2];
    pipe(fd);

    // Creates the child process to execute the command
    if (fork() == 0) {
      // Redirects the stdin of the child process
      dup2(in_fd, STDIN_FILENO);

      // Redirects the stdout of the child process
      if (i < number_of_commands - 1) {
        dup2(fd[1], STDOUT_FILENO);
      } else {
        // To the last command, redirects the output to the specified file
        out_fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);

        // Verifies if the file was opened successfully
        if (out_fd < 0) {
          perror("open");
          exit(EXIT_FAILURE);
        }

        // Redirects the stdout to the output file
        dup2(out_fd, STDOUT_FILENO);

        // Closes the output file descriptor
        close(out_fd);
      }

      // Closes the file descriptors not used by the child process
      close(fd[0]);  // Fecha o lado de leitura do pipe não utilizado pelo filho

      // Executes the command
      exec_command(commands[i], number_of_commands);

      // Exits the child process, exec_command only returns if there is an error
      _exit(EXIT_FAILURE);
    } else {
      // Waits for the child process to finish
      wait(NULL);

      // Closes the writing side of the pipe in the parent process
      close(fd[1]);

      // The next command will use the output of this pipe as input
      in_fd = fd[0];
    }
  }

  return 0;
}

int count_commands(char* program) {
  // Starts the counter with 1, since there is at least one command
  int count = 1;

  // Pointer to the first ocurrence of the pipe character
  const char* tmp = program;

  // Counts the number of commands in the program string
  while ((tmp = strchr(tmp, '|')) != NULL) {
    count++;
    tmp++;
  }

  return count;
}

void split_commands(
    char* program, char** task_commands, int number_of_commands
) {
  // Delimiter used to split the program string
  const char delim[2] = "|";

  // Token to store the current command
  char* token;

  // Splits the program string into an array of commands
  token = strtok(program, delim);

  // Index of the current command
  int i = 0;

  // Stores each command in the array of commands
  while (token != NULL && i < number_of_commands) {
    task_commands[i] = token;
    token = strtok(NULL, delim);
    i++;
  }
}


void write_id(int id){
  char write_tak_id[50];
  sprintf(write_tak_id, "Task received has id: %d\n", id);
  write(1, write_tak_id, strlen(write_tak_id));

}
