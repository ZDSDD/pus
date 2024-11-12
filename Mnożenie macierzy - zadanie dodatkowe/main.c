#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "utilMatrix.h"
#include "signalsVariant.h"

int maxProcessLimit = 0x0;

int generate_random_int(int a, int b)
{
    // Generate a random number between 0 and (b - a + 1)
    int random_num = rand() % (b - a + 1);

    // Shift the range to [a, b]
    return a + random_num;
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

void userInterface(int *r1, int *r2, int *c1, int *c2)
{
    printf("Enter rows and columns for the first matrix: ");
    scanf("%d %d", r1, c1);
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
        }
        else if (strcmp(argv[i], "--p-limit") == 0)
        {
            maxProcessLimit = 0x1;
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

    generateRandomMatrix(first, row1, col1);
    printf("Generated first matrix:\n");
    printMatrix(first, row1, col1);
    generateRandomMatrix(second, row2, col2);
    printf("Generated second matrix:\n");
    printMatrix(second, row2, col2);

    foo(first, second, row1, col1, col2, 0x0);

    // Free allocated memory
    for (int i = 0; i < row1; i++)
        free(first[i]);
    free(first);

    for (int i = 0; i < row2; i++)
        free(second[i]);
    free(second);

    return 0;
}