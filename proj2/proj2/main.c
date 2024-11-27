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
void verify_results(int **first, int **second, int rows, int cols)
{
    int result_match = 1;
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            if (first[i][j] != second[i][j])
            {
                result_match = 0;
                break;
            }
        }
        if (!result_match)
            break;
    }

    printf("Wyniki obu metod %s\n", result_match ? "ZGODNE" : "RÓŻNE");
}

void multiplyMatricesUsingPipes(int **first, int **second, int **result, int row1, int common_dim, int col2)
{
    int i, j, k;

    // Create pipes for communication
    int pipes[row1][col2][2]; // One pipe for each cell in the result matrix

    for (i = 0; i < row1; i++)
    {
        for (j = 0; j < col2; j++)
        {
            if (pipe(pipes[i][j]) == -1)
            {
                perror("pipe");
                exit(EXIT_FAILURE);
            }
        }
    }

    // Create child processes
    for (i = 0; i < row1; i++)
    {
        for (j = 0; j < col2; j++)
        {
            pid_t pid = fork();

            if (pid == -1)
            {
                perror("fork");
                exit(EXIT_FAILURE);
            }

            if (pid == 0)
            {                          // Child process
                close(pipes[i][j][0]); // Close read end of the pipe in the child

                // Compute the cell value
                int cellValue = 0;
                for (k = 0; k < common_dim; k++)
                {
                    cellValue += first[i][k] * second[k][j];
                }

                // Write the result to the pipe
                if (write(pipes[i][j][1], &cellValue, sizeof(int)) == -1)
                {
                    perror("write");
                    exit(EXIT_FAILURE);
                }

                close(pipes[i][j][1]); // Close write end of the pipe
                exit(0);               // Terminate child process
            }
        }
    }

    // Parent process: Gather results
    for (i = 0; i < row1; i++)
    {
        for (j = 0; j < col2; j++)
        {
            close(pipes[i][j][1]); // Close write end of the pipe in the parent

            // Read the result from the pipe
            if (read(pipes[i][j][0], &result[i][j], sizeof(int)) == -1)
            {
                perror("read");
                exit(EXIT_FAILURE);
            }

            close(pipes[i][j][0]); // Close read end of the pipe
        }
    }

    // Wait for all child processes to complete
    for (i = 0; i < row1; i++)
    {
        for (j = 0; j < col2; j++)
        {
            wait(NULL);
        }
    }
}

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

int main(int argc, char *argv[])
{
    int rows = 1000, cols = 1000, common_dim = 100;

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
    // Wait for all child processes to complete
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            wait(NULL);
        }
    }

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
    clock_gettime(CLOCK_MONOTONIC, &start);
    multiplyMatricesUsingPipes(A, B, result_pipe, rows, common_dim, cols);
    // Odczekaj na zakończenie wszystkich procesów potomnych
    printf("zaczybamy zczekac\n");
    while (wait(NULL) > 0)
        ;
    printf("skonczyule czekac\n");
    clock_gettime(CLOCK_MONOTONIC, &end);
    double time_pipe = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    // Dodatkowa weryfikacja wyniku

    int **result_true = allocateMatrix(rows, cols);
    multiplyMatrices(A, B, result_true, rows, common_dim, cols);
    // printf("\n");
    // printMatrix(result_true, rows, cols);
    // printf("\n");
    // printMatrix(result_pipe, rows, cols);
    // printf("\n");
    // printMatrix(result_shared_memory, rows, cols);
    // printf("\n");
    printf("Wyniki dla pipe & result true: \n");
    verify_results(result_pipe, result_true, rows, cols);
    printf("Wyniki dla shared_memory & result true: \n");
    verify_results(result_shared_memory, result_true, rows, cols);
    printf("Wyniki dla shared_memory & pipe: \n");
    verify_results(result_shared_memory, result_pipe, rows, cols);
    printf("Czas (pamięć współdzielona): %.6f sekund\n", time_shared_memory);
    printf("Czas (pipes): %.6f sekund\n", time_pipe);

    // Free allocated memory
    freeMatrix(A, rows);
    freeMatrix(B, common_dim);
    freeMatrix(result_shared_memory, rows);
    freeMatrix(result_pipe, rows);
    freeMatrix(result_true, rows);
    return 0;
}