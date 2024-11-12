#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "signals_seba.h"
#include "utilMatrix.h"

#define MAX_COL_VALUE 0xFF
#define MAX_ROW_VALUE 0xFF
#define MAX_VALUE 0xFFFF

#define SIG_RESULT (SIGRTMIN)        // Real-time signal for result transfer
volatile int completed_elements = 0; // Counter for completed elements
int **result;                        // Pointer to store result matrix in the parent

int generate_random_int(int a, int b)
{
    // Generate a random number between 0 and (b - a + 1)
    int random_num = rand() % (b - a + 1);

    // Shift the range to [a, b]
    return a + random_num;
}

// Signal handler to decode the signal and update the result matrix
void handle_sig_result(int sig, siginfo_t *info, void *ucontext)
{
    int value = info->si_value.sival_int;

    int row = (value >> 24) & 0xFF;
    int col = (value >> 16) & 0xFF;
    int result_value = value & 0xFFFF;

    result[row][col] = result_value;
    completed_elements++;
}

void printBinary(int number)
{
    // Iterate through each bit, starting from the most significant bit (31) down to 0
    for (int i = 31; i >= 0; i--)
    {
        // Use a bitwise AND to check if the bit at position i is 1 or 0
        int bit = (number >> i) & 1;
        printf("%d", bit);

        // Print a space every 4 bits for readability
        if (i % 4 == 0)
        {
            printf(" ");
        }
    }
    printf("\n");
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

void multiplyMatricesSignalVariant(int **first, int **second, int **result, int row1, int col1, int col2)
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

void userInterface(int *r1, int *r2, int *c1, int *c2)
{
    // Get dimensions for the first matrix
    printf("Enter rows and columns for the first matrix: ");
    scanf("%d %d", r1, c1);
    // Get dimensions for the second matrix
    printf("Enter rows and columns for the second matrix: ");
    scanf("%d %d", r2, c2);
}

int main(int argc, char *argv[])
{
    int row1, col1, row2, col2;

    srand(time(NULL)); // Seed the random number generator

    int userInput = 0; // Flag
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--user") == 0)
        {
            userInput = 1;
            break;
        }
    }

    if (userInput == 0)
    {
        row1 = generate_random_int(3, 3);
        row2 = generate_random_int(3, 3);
        col1 = row2;
        col2 = generate_random_int(3, 3);
    }
    else
    {
        userInterface(&row1, &row2, &col1, &col2);
    }

    // Check if matrix multiplication is possible
    if (col1 != row2)
    {
        printf("Matrix multiplication not possible. Number of columns in the first matrix must equal number of rows in the second matrix.\n");
        return -1;
    }
    // Allocate memory for the first matrix
    int **first = (int **)malloc(row1 * sizeof(int *));
    for (int i = 0; i < row1; i++)
    {
        first[i] = (int *)malloc(col1 * sizeof(int));
    }
    // Allocate memory for the second matrix
    int **second = (int **)malloc(row2 * sizeof(int *));
    for (int i = 0; i < row2; i++)
    {
        second[i] = (int *)malloc(col2 * sizeof(int));
    }
    // Allocate memory for the result matrix
    result = (int **)malloc(row1 * sizeof(int *));
    for (int i = 0; i < row1; i++)
    {
        result[i] = (int *)malloc(col2 * sizeof(int));
    }

    generateRandomMatrix(first, row1, col1);
    printf("Generated first matrix:\n");
    printMatrix(first, row1, col1);
    generateRandomMatrix(second, row2, col2);
    printf("Generated second matrix:\n");
    printMatrix(second, row2, col2);

    // Set up the signal handler for the real-time signal in the parent process
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = handle_sig_result;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIG_RESULT, &sa, NULL) == -1)
    {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
    multiplyMatricesSignalVariant(first, second, result, row1, col1, col2);
    // Wait until all elements are computed
    while (completed_elements < row1 * col2)
    {
        pause(); // Wait for signals from children
    }

    printf("Result of matrix multiplication:\n");
    printMatrix(result, row1, col2);

    // Free allocated memory
    for (int i = 0; i < row1; i++)
        free(first[i]);
    free(first);

    for (int i = 0; i < row2; i++)
        free(second[i]);
    free(second);

    for (int i = 0; i < row1; i++)
        free(result[i]);
    free(result);
    return 0;
}