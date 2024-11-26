#include <unistd.h> // For fork
#include <stdio.h>  // For perror
#include <stdlib.h> // For exit
#include <signal.h> // For sigqueue and union sigval

#include <time.h>
#include "signals_seba.h"
#include "utilMatrix.h"

#define MAX_COL_VALUE 0xFF
#define MAX_ROW_VALUE 0xFF
#define MAX_VALUE 0xFFFF
#define MAX_PROCESSES 10             // Maximum number of processes
#define SIG_RESULT (SIGRTMIN)        // Real-time signal for result transfer
volatile int completed_elements = 0; // Counter for completed elements
int **result;                        // Pointer to store result matrix in the parent
volatile int process_count = 0;

// Signal handler to decode the signal and update the result matrix
void handle_sig_result(int sig, siginfo_t *info, void *ucontext)
{
    int value = info->si_value.sival_int;

    int row = (value >> 24) & 0xFF;
    int col = (value >> 16) & 0xFF;
    int result_value = value & 0xFFFF;

    result[row][col] = result_value;
    process_count--;
}

int encode(int col, int row, int val)
{
    if (val > 0xFFFF)
    {
        printf("val to high %d\n", val);
        val = -1;
    }
    if (col > MAX_COL_VALUE)
    {
        printf("col value out of bounds");
        exit(EXIT_FAILURE);
    }
    if (col > MAX_ROW_VALUE)
    {
        printf("row value out of bounds");
        exit(EXIT_FAILURE);
    }
    int encoded_val = 0;
    // printf("start encoding... for row: %d, col: %d, val: %d\n", row, col, val);
    // printBinary(encoded_val);
    // encoded_val = encoded_val | (row) << 16;
    // printBinary(encoded_val);
    // encoded_val = encoded_val | (col) << 24;
    // printBinary(encoded_val);
    // encoded_val = encoded_val | val;
    // printBinary(encoded_val);
    // return encoded_val;
    return encoded_val | (row << 24) | (col << 16) | val;
}

void multiplyMatricesSignalVariant(int **first, int **second, int row1, int col1, int col2)
{
    for (int i = 0; i < row1; i++)
    {
        for (int j = 0; j < col2; j++)
        {
            pid_t pid = fork();
            if (pid == -1)
            {
                perror("fork");
                exit(EXIT_FAILURE);
            }
            else if (pid == 0)
            { // Child process
                // Calculate result[i][j]
                int result_ij = 0;
                for (int k = 0; k < col1; k++)
                {
                    result_ij += first[i][k] * second[k][j];
                }

                int value = encode(j, i, result_ij);

                // Send signal with the encoded value
                union sigval sv;
                sv.sival_int = value;
                sigqueue(getppid(), SIG_RESULT, sv);

                _exit(0); // Exit the child process
            }
        }
    }
}
void multiplyMatricesSignalVariantWithProcessLimit(int **first, int **second, int row1, int col1, int col2)
{
    int status;
    pid_t pid;
    for (int i = 0; i < row1; i++)
    {
        for (int j = 0; j < col2; j++)
        {
            while (process_count >= MAX_PROCESSES)
            {
                while ((pid = waitpid(-1, &status, NULL)) > 0)
                {
                }
            }

            process_count++; // Increment the process count
            pid_t pid = fork();
            if (pid == -1)
            {
                perror("fork");
                exit(EXIT_FAILURE);
            }
            else if (pid == 0) // Child process
            {
                // Calculate result[i][j]
                int result_ij = 0;
                for (int k = 0; k < col1; k++)
                {
                    result_ij += first[i][k] * second[k][j];
                }

                int value = encode(j, i, result_ij);

                // Send signal with the encoded value
                union sigval sv;
                sv.sival_int = value;
                sigqueue(getppid(), SIG_RESULT, sv);

                _exit(0); // Exit the child process
            }
        }
    }
}
void foo(int **first, int **second, int row1, int col1, int col2, int p_limit_flag)
{
    // Set up the signal handler for the real-time signal in the parent process
    // Allocate memory for the result matrix
    result = (int **)malloc(row1 * sizeof(int *));
    for (int i = 0; i < row1; i++)
    {
        result[i] = (int *)malloc(col2 * sizeof(int));
    }
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = handle_sig_result;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIG_RESULT, &sa, NULL) == -1)
    {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
    if (p_limit_flag)
    {
        printf("Using signal version with process limit to %d\n", MAX_PROCESSES);
        multiplyMatricesSignalVariantWithProcessLimit(first, second, row1, col1, col2);
    }
    else
    {
        printf("using signal variant with no process limit\n");
        multiplyMatricesSignalVariant(first, second, row1, col1, col2);
    }
    int status;
    pid_t wpid;
    while ((wpid = wait(&status)) > 0)
        ;

    printf("Result of matrix multiplication:\n");
    printMatrix(result, row1, col2);
    for (int i = 0; i < row1; i++)
        free(result[i]);
    free(result);
}