#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>

#define FIFO_NAME "task_fifo"

int main() {
    int fd;
    char task[256];

    fd = open(FIFO_NAME, O_WRONLY);

    strcpy(task, "Example of task from client");
    write(fd, task, strlen(task) + 1);

    close(fd);

    return 0;
}
