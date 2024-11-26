#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "utilMatrix.h"
#include "signalsVariant.h"
#include "fileVariant.h"

int maxProcessLimit = 0x0;

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

void userInterface(int *r1, int *r2, int *c1, int *c2, int *l, int *u)
{
    printf("Enter rows and columns for the first matrix: ");
    scanf("%d %d", r1, c1);
    printf("Enter rows and columns for the second matrix: ");
    scanf("%d %d", r2, c2);
    printf("Enter lower and upper bound for numbers in the matrix: ");
    scanf("%d %d", l, u);
}
enum algo_variant
{
    SIGNALS,
    FILES
} av = SIGNALS;
int main(int argc, char *argv[])
{
    int row1, col1, row2, col2, lb, ub;

    srand(time(NULL)); // Seed the random number generator

    int userInput = 0; // Flag
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--user") == 0)
        {
            userInput = 1;
        }
        else if (strcmp(argv[i], "--p-limit") == 0)
        {
            maxProcessLimit = 0x1;
        }
        else if (strcmp(argv[i], "--file-variant") == 0)
        {
            av = FILES;
        }
    }

    if (userInput == 0)
    {
        row1 = generate_random_int(3, 3);
        row2 = generate_random_int(3, 3);
        col1 = row2;
        col2 = generate_random_int(3, 3);
        lb = generate_random_int(0, 10);
        ub = generate_random_int(6, 100);
    }
    else
    {
        userInterface(&row1, &row2, &col1, &col2, &lb, &ub);
    }

    // Check if matrix multiplication is possible
    if (col1 != row2)
    {
        printf("Matrix multiplication not possible. Number of columns in the first matrix must equal number of rows in the second matrix.\n");
        return -1;
    }
    int **first = allocateMatrix(row1, col1);
    int **second = allocateMatrix(row2, col2);

    generateRandomMatrix(first, row1, col1, lb, ub);
    printf("Generated first matrix:\n");
    printMatrix(first, row1, col1);
    generateRandomMatrix(second, row2, col2, lb, ub);
    printf("Generated second matrix:\n");
    printMatrix(second, row2, col2);

    if (av == SIGNALS)
    {
        foo(first, second, row1, col1, col2, maxProcessLimit);
    }
    else if (av == FILES)
    {
        int **res = allocateMatrix(row2, col2);
        printf("using file variant\n");
        multiplyMatricesFileWariant(first, second, res, row1, col1, col2);
        printMatrix(res, row1, col2);
        freeMatrix(res, row1);
    }

    // Free allocated memory
    freeMatrix(first, row1);
    freeMatrix(second, row2);

    return 0;
}