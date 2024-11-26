#ifndef utilMatrix
#define utilMatrix

// Helper function to allocate 2D matrix
int **allocateMatrix(int rows, int cols)
{
    int **matrix = malloc(rows * sizeof(int *));
    for (int i = 0; i < rows; i++)
    {
        matrix[i] = malloc(cols * sizeof(int));
    }
    return matrix;
}

// Helper function to free 2D matrix
void freeMatrix(int **matrix, int rows)
{
    for (int i = 0; i < rows; i++)
    {
        free(matrix[i]);
    }
    free(matrix);
}

void printMatrix(int **matrix, int rows, int cols)
{
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            printf("%d ", matrix[i][j]);
        }
        printf("\n");
    }
}
int generate_random_int(int a, int b)
{
    // Generate a random number between 0 and (b - a + 1)
    int random_num = rand() % (b - a + 1);

    // Shift the range to [a, b]
    return a + random_num;
}
void generateRandomMatrix(int **matrix, int rows, int cols, int lower_bound, int upper_bound)
{
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            matrix[i][j] = generate_random_int(lower_bound, upper_bound);
        }
    }
}

void multiplyMatrices(int **first, int **second, int **result, int row1, int col1, int col2)
{
    for (int i = 0; i < row1; i++)
    {
        for (int j = 0; j < col2; j++)
        {
            result[i][j] = 0;
            for (int k = 0; k < col1; k++)
            {
                result[i][j] += first[i][k] * second[k][j];
            }
        }
    }
}
#endif