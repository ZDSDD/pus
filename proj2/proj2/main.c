#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>
#include "utilMatrix.h"
#include <errno.h>
#include <sys/types.h> // Defines data types used in System V IPC
#include <sys/ipc.h>   // Defines IPC macros and structures
#include <sys/shm.h>   // Defines shared memory operations

#define MAX_CONCURRENT 5 // Max number of processes to run at the same time

#define SHM_KEY 12345

void calculate_element_shared_memory(int **A, int **B, int row, int col, int common_dim, int shm_id, int cols)
{
    int *shared_memory = (int *)shmat(shm_id, NULL, 0);
    if (shared_memory == (void *)-1)
    {
        perror("shmat");
        exit(EXIT_FAILURE);
    }

    int result = 0;
    for (int k = 0; k < common_dim; k++)
    {
        result += A[row][k] * B[k][col];
    }

    shared_memory[row * cols + col] = result;

    if (shmdt(shared_memory) == -1)
    {
        perror("shmdt failed");
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}
typedef struct Data
{
    int row;
    int col;
    int value;
} data_t;

void calculate_element_pipe(int **A, int **B, int row, int col, int common_dim, int write_fd)
{
    int result = 0;
    for (int k = 0; k < common_dim; k++)
    {
        result += A[row][k] * B[k][col];
    }

    data_t toSend = {row, col, result};

    // Use a loop with error handling for writing
    ssize_t bytes_written;
    bytes_written = write(write_fd, &toSend, sizeof(data_t));
    if (bytes_written == -1)
    {
        perror("write failed");
        close(write_fd);
        exit(EXIT_FAILURE);
    }

    close(write_fd);
    exit(EXIT_SUCCESS);
}
/* procedura obslugi sygnalu SIGCLD */
void handler_sigcld(int sig)
{
    int status;
    while (waitpid(-1, &status, WNOHANG) > 0)
        ;
}
int main(int argc, char *argv[])
{
    /* instalacja procedury obslugi sygnalu SIGCLD */
    signal(SIGCLD, handler_sigcld);

    int rows = 100, cols = 100, common_dim = 100;

    int **A = allocateMatrix(rows, common_dim);
    int **B = allocateMatrix(common_dim, cols);
    int **result_shared_memory = allocateMatrix(rows, cols);
    int **result_pipe = allocateMatrix(rows, cols);

    generateRandomMatrix(A, rows, common_dim, 1, 10);
    generateRandomMatrix(B, common_dim, cols, 1, 10);

    // SHARED MEMORY !!! //
    int shm_id = shmget(SHM_KEY, rows * cols * sizeof(int), IPC_CREAT | 0666);
    if (shm_id < 0)
    {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    // Time benchmark
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            pid_t pid = fork();
            if (pid == 0) // Child process
            {
                calculate_element_shared_memory(A, B, i, j, common_dim, shm_id, cols);
            }
            else if (pid < 0) // Fork failed
            {
                perror("Fork failed");
            }
        }
    }

    // wait for all processess to end
    printf("czekamy...\n");
    // Wait for all processes
    while (wait(NULL) > 0)
        ;

    printf("procesy z pamiecia wspoldezielona zakonczony\n");

    clock_gettime(CLOCK_MONOTONIC, &end);
    double time_shared_memory = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    // Read from shared memory
    int *shared_memory = (int *)shmat(shm_id, NULL, 0);
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            result_shared_memory[i][j] = shared_memory[i * cols + j];
        }
    }
    shmdt(shared_memory);
    shmctl(shm_id, IPC_RMID, NULL); // clear shared memory

    // PIPES //
    int pipe_fd[2];
    if (pipe(pipe_fd) == -1)
    {
        perror("Błąd tworzenia potoku");
        exit(EXIT_FAILURE);
    }

    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            pid_t pid = fork();
            if (pid == 0)
            {
                close(pipe_fd[0]); // Zamknij odczyt w procesach potomnych
                calculate_element_pipe(A, B, i, j, common_dim, pipe_fd[1]);
            }
            else if (pid < 0)
            {
                perror("Fork failed");
                break;
            }
        }
    }

    close(pipe_fd[1]); // Zamknij zapis w procesie macierzystym

    // wait for all processess to end
    printf("czekamy...\n");
    // Wait for pipe processes
    while (wait(NULL) > 0)
        ;

    printf("procesy z pipem zakonczone!\n");

    data_t data;
    close(pipe_fd[1]); // Zamykamy stronę zapisu
    while (read(pipe_fd[0], &data, sizeof(data)) > 0)
    {
        result_pipe[data.row][data.col] = data.value;
    }
    close(pipe_fd[0]);

    clock_gettime(CLOCK_MONOTONIC, &end);
    double time_pipe = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    // Dodatkowa weryfikacja wyniku
    int result_match = 1;
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            if (result_shared_memory[i][j] != result_pipe[i][j])
            {
                result_match = 0;
                break;
            }
        }
        if (!result_match)
            break;
    }
    printf("Wyniki obu metod %s\n", result_match ? "ZGODNE" : "RÓŻNE");
    // Print resultss
    // printf("Wynik (pamięć współdzielona):\n");
    // printMatrix(result_shared_memory, rows, cols);
    printf("Czas (pamięć współdzielona): %.6f sekund\n", time_shared_memory);

    // printf("Wynik (pipes):\n");
    // printMatrix(result_pipe, rows, cols);
    printf("Czas (pipes): %.6f sekund\n", time_pipe);

    // Free allocated memory
    freeMatrix(A, rows);
    freeMatrix(B, common_dim);
    freeMatrix(result_shared_memory, rows);
    freeMatrix(result_pipe, rows);
    return 0;
}