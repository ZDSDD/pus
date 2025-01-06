// Server Program in C
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <stdbool.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 10

typedef struct
{
    int start;
    int end;
} Range;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

bool is_prime(int n)
{
    if (n < 2)
        return false;
    for (int i = 2; i * i <= n; i++)
    {
        if (n % i == 0)
            return false;
    }
    return true;
}

void *handle_client(void *arg)
{
    int client_socket = *(int *)arg;
    free(arg);

    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);

    // Receive range from client
    read(client_socket, buffer, BUFFER_SIZE);

    Range range;
    sscanf(buffer, "%d %d", &range.start, &range.end);

    printf("[Server] Received range: [%d, %d]\n", range.start, range.end);

    int prime_count = 0;
    for (int i = range.start; i <= range.end; i++)
    {
        if (is_prime(i))
        {
            prime_count++;
        }
    }

    // Send result back to client
    memset(buffer, 0, BUFFER_SIZE);
    snprintf(buffer, BUFFER_SIZE, "Found %d primes in range [%d, %d]", prime_count, range.start, range.end);
    write(client_socket, buffer, strlen(buffer));

    printf("[Server] Sent result to client: %s\n", buffer);

    close(client_socket);
    return NULL;
}

int main()
{
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    // Create socket
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // Bind socket
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Bind failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Listen for connections
    if (listen(server_socket, MAX_CLIENTS) < 0)
    {
        perror("Listen failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("[Server] Listening on port %d\n", PORT);

    while (1)
    {
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_socket < 0)
        {
            perror("Accept failed");
            continue;
        }

        int *client_socket_ptr = malloc(sizeof(int));
        *client_socket_ptr = client_socket;
        printf("[Server] Connected to client. Client socket:[%d]\n", client_socket);
        pthread_t thread;
        pthread_create(&thread, NULL, handle_client, client_socket_ptr);
        pthread_detach(thread);
    }

    close(server_socket);
    return 0;
}
