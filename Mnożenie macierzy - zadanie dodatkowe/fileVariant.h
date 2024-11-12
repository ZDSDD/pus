#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>

#define RESULT_FILE "matrix_result.txt"

// Structure to store the result of a single cell computation
typedef struct {
    int row;
    int col;
    int value;
} CellResult;

void computeCell(int **first, int **second, int row, int col, int col1, int fd) {
    int result = 0;
    for (int k = 0; k < col1; k++) {
        result += first[row][k] * second[k][col];
    }
    
    CellResult cellResult = {row, col, result};
    
    // Use write with file locking to ensure atomic writes
    struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    
    fcntl(fd, F_SETLKW, &lock);
    write(fd, &cellResult, sizeof(CellResult));
    
    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lock);
}

void multiplyMatricesFileWariant(int **first, int **second, int **result, int row1, int col1, int col2) {
    // Create/truncate the result file
    int fd = open(RESULT_FILE, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        perror("Error opening file");
        exit(1);
    }
    
    int total_processes = row1 * col2;
    pid_t *pids = malloc(total_processes * sizeof(pid_t));
    int process_count = 0;
    
    // Create processes for each cell
    for (int i = 0; i < row1; i++) {
        for (int j = 0; j < col2; j++) {
            pids[process_count] = fork();
            
            if (pids[process_count] == -1) {
                perror("Fork failed");
                exit(1);
            }
            
            if (pids[process_count] == 0) { // Child process
                computeCell(first, second, i, j, col1, fd);
                close(fd);
                exit(0);
            }
            process_count++;
        }
    }
    
    // Wait for all child processes to complete
    for (int i = 0; i < total_processes; i++) {
        waitpid(pids[i], NULL, 0);
    }
    
    close(fd);
    
    // Read results from file and populate result matrix
    fd = open(RESULT_FILE, O_RDONLY);
    if (fd == -1) {
        perror("Error opening file for reading");
        exit(1);
    }
    
    CellResult cellResult;
    while (read(fd, &cellResult, sizeof(CellResult)) == sizeof(CellResult)) {
        result[cellResult.row][cellResult.col] = cellResult.value;
    }
    
    close(fd);
    free(pids);
}