#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#define FIFO_NAME "task_fifo"

int main() {
    int fd;
    char task[256];

    mkfifo(FIFO_NAME, 0666);

    fd = open(FIFO_NAME, O_RDONLY);

    while (read(fd, task, sizeof(task)) > 0) {
        printf("Received task: %s\n", task);
    }

    close(fd);

    return 0;
}
