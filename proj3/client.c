// Client Program in C
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <start> <end>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int start = atoi(argv[1]);
    int end = atoi(argv[2]);

    int client_socket;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    // Create socket
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Socket creation failed");
        return EXIT_FAILURE;
    }

    // Set server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0)
    {
        perror("Invalid address");
        close(client_socket);
        return EXIT_FAILURE;
    }

    // Connect to server
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Connection failed");
        close(client_socket);
        return EXIT_FAILURE;
    }

    // Send range to server
    snprintf(buffer, BUFFER_SIZE, "%d %d", start, end);
    write(client_socket, buffer, strlen(buffer));

    // Receive result from server
    memset(buffer, 0, BUFFER_SIZE);
    read(client_socket, buffer, BUFFER_SIZE);
    printf("[Client] Server response: %s\n", buffer);

    close(client_socket);
    return 0;
}