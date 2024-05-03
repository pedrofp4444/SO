#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#ifndef UTILS_H
#define UTILS_H

// Defines the client fifo path and name
#define CLIENT "tmp/client_fifo"

// Defines the orchestrator fifo path and name
#define ORCHESTRATOR "tmp/orchestrator_fifo"

// Defines the maximum path for the output file
#define PATH_MAX 4096

typedef enum phase {
  EXECUTING,  // 0
  SCHEDULED,  // 1
  COMPLETED,  // 2
} Phase;

typedef enum type {
  STATUS,   // 0
  EXECUTE,  // 1
} Type;

typedef struct {
  Phase type;
  int id;
  char program[308];
} METRICS;

// Enum to indicate the type of the task

// Struct to represent a task
typedef struct {
  Type type;
  char program[308];
  int duration;
  int id;
  pid_t pid;
  struct timeval start_time;
} Task;

/**
 * Creates a FIFO with the given name
 * 
 * @param name The name of the FIFO
 * @return 1 if the FIFO was created successfully, 0 otherwise
 */
int createFIFO(char* name);

/**
 * Opens a FIFO with the given name
 * 
 * @param name The name of the FIFO
 * @param mode The mode to open the FIFO
 * @return The file descriptor of the FIFO if it was opened successfully, 0 otherwise
 */
int openFIFO(char* name, int mode);

/**
 * Executes a command
 * 
 * @param arg The command to be executed
  * @param number_of_commands The number of commands
 * @return 0 if the command was executed successfully, -1 otherwise
 */
int exec_command(char* arg, int number_of_commands);

/**
 * Executes a task
 * 
 * @param number_of_commands The number of commands in the task
 * @param commands The commands to be executed
 * @param output_file The output file path
 * @return 0 if the task was executed successfully, -1 otherwise
 */
int execute_task(int number_of_commands, char** commands, char* output_file);

/**
 * Counts the number of commands in a program string
 * 
 * @param program The program string
 * @return The number of commands in the program string
 */
int count_commands(char* program);

/**
 * Splits the program string into an array of commands
 * 
 * @param program The program string
 * @param task_commands The array of commands
 * @param number_of_commands The number of commands in the program string
 */
void split_commands(
    char* program, char** task_commands, int number_of_commands
);

/**
 * Writes the task id to client
 * 
 * @param id The id to be written
 * 
*/
void write_id(int id);

#endif
