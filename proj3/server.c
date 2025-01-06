#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <arpa/inet.h>
#include <stdbool.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 10
#define INITIAL_CACHE_SIZE 1000000

typedef struct
{
    int start;
    int end;
} Range;

typedef struct
{
    int *prefix_sum;
    bool *is_calculated;
    size_t current_size;
    pthread_mutex_t lock;
    bool initialized;
    int highest_calculated;
} PrimeCache;

PrimeCache cache = {NULL, NULL, 0, PTHREAD_MUTEX_INITIALIZER, false};

bool is_prime(int n)
{
    if (n < 2)
        return false;
    for (int i = 2; i * i <= n; i++)
        if (n % i == 0)
            return false;
    return true;
}

void resize_cache(size_t new_size)
{
    printf("[Server] Resizing cache to %zu\n", new_size);
    int *new_prefix_sum = realloc(cache.prefix_sum, new_size * sizeof(int));
    bool *new_is_calculated = realloc(cache.is_calculated, new_size * sizeof(bool));

    if (!new_prefix_sum || !new_is_calculated)
    {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    // Initialize new memory
    memset(new_is_calculated + cache.current_size, 0,
           (new_size - cache.current_size) * sizeof(bool));

    cache.prefix_sum = new_prefix_sum;
    cache.is_calculated = new_is_calculated;
    cache.current_size = new_size;
}

void init_cache()
{
    pthread_mutex_lock(&cache.lock);
    if (!cache.initialized)
    {
        cache.prefix_sum = calloc(INITIAL_CACHE_SIZE, sizeof(int));
        cache.is_calculated = calloc(INITIAL_CACHE_SIZE, sizeof(bool));
        if (!cache.prefix_sum || !cache.is_calculated)
        {
            fprintf(stderr, "Initial memory allocation failed\n");
            exit(EXIT_FAILURE);
        }
        cache.current_size = INITIAL_CACHE_SIZE;
        cache.initialized = true;
        cache.highest_calculated = -1; // Initialize to -1 (nothing calculated yet)
    }
    pthread_mutex_unlock(&cache.lock);
}
void calculate_cache(int end)
{
    pthread_mutex_lock(&cache.lock);

    // Resize if needed
    if (end >= cache.current_size)
    {
        size_t new_size = end + 1;
        resize_cache(new_size);
    }

    // Start calculating from the next uncached index
    int start = cache.highest_calculated + 1;
    for (int i = start; i <= end; i++)
    {
        cache.prefix_sum[i] = (i > 0 ? cache.prefix_sum[i - 1] : 0) +
                              (is_prime(i) ? 1 : 0);
        cache.is_calculated[i] = true;
    }

    // Update the highest calculated index
    if (end > cache.highest_calculated)
        cache.highest_calculated = end;

    pthread_mutex_unlock(&cache.lock);
}

int get_prime_count(int start, int end)
{
    if (start < 0)
        start = 0;
    calculate_cache(end);

    pthread_mutex_lock(&cache.lock);
    int result = cache.prefix_sum[end] - (start > 0 ? cache.prefix_sum[start - 1] : 0);
    pthread_mutex_unlock(&cache.lock);

    return result;
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
    if (range.start > range.end || range.start < 0 || range.end < 0)
    {
        snprintf(buffer, BUFFER_SIZE, "Invalid range [%d, %d].", range.start, range.end);
        write(client_socket, buffer, strlen(buffer));
        printf("[Server] Sent error to client: %s\n", buffer);
        close(client_socket);
        return NULL;
    }

    // Use cached prime counting
    int prime_count = get_prime_count(range.start, range.end);

    // Send result back to client
    memset(buffer, 0, BUFFER_SIZE);
    snprintf(buffer, BUFFER_SIZE, "Found %d primes in range [%d, %d]", prime_count, range.start, range.end);
    write(client_socket, buffer, strlen(buffer));

    printf("[Server] Sent result to client: %s\n", buffer);

    close(client_socket);
    return NULL;
}

void cleanup_cache()
{
    pthread_mutex_lock(&cache.lock);
    free(cache.prefix_sum);
    free(cache.is_calculated);
    pthread_mutex_unlock(&cache.lock);
}

void signal_handler(int signum)
{
    printf("[Server] Received signal %d, shutting down...\n", signum);
    cleanup_cache();
    exit(0);
}

int main()
{
    signal(SIGINT, signal_handler);
    init_cache();

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