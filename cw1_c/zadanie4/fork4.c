#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

int main() {
    char c;
    int pid = fork();

    int fd = open("fork4.c", O_RDONLY);

    if (fd < 0) {
        perror("Failed to open the file");
        return 1;
    }

    while (read(fd, &c, 1) == 1) {
        if (pid > 0) {
            printf("Parent: %c\n", c);
        } else {
            printf("Child: %c\n", c);
        }
        fflush(stdout);
        sleep(1);
    }

    close(fd);
    return 0;
}