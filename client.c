//Multi client connet to server
#include "common.h"
 
#define SERVER_NAME_LEN_MAX 255
#define BUFFER_SIZE 1024
 
 
void start_listen_in_new_thread(int socket_fd);
 
 
void* listen_and_print(void* arg) {
    int socket_fd = *(int *)arg;
    free(arg); // Free memory after usage
 
    char buffer[BUFFER_SIZE];
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int amount_received = recv(socket_fd, buffer, BUFFER_SIZE, 0);
        if (amount_received > 0) {
            printf("Message from server: %s\n", buffer);
        } else if (amount_received == 0) {
            printf("Server closed the connection.\n");
            break;
        } else {
            perror("recv");
            break;
        }
    }
 
    close(socket_fd);
    return NULL;
}
 
int main(int argc, char *argv[]) {
    char server_name[SERVER_NAME_LEN_MAX + 1] = { 0 };
    int socket_fd;
    struct sockaddr_in server_address;
    char message[1024];
 
    // /* Get server name from command line arguments or stdin. */
    if (argc > 1) {
        strncpy(server_name, argv[1], SERVER_NAME_LEN_MAX);
    } else {
        printf("Enter Server Name: ");
        scanf("%s", server_name);
    }
 
    /* Get server port from command line arguments or stdin. */
    int server_port = argc > 2 ? atoi(argv[2]) : 0;
    if (!server_port) {
        printf("Enter Port: ");
        scanf("%d", &server_port);
    }
 
    memset(&server_address, 0, sizeof server_address);
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(server_port);   
    if (inet_pton(AF_INET, server_name, &server_address.sin_addr.s_addr) <= 0) {
    perror("inet_pton error");
    exit(EXIT_FAILURE);
    }
 
    /* Create TCP socket. */
    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }
 
    /* Connect to socket with server address. */
    if (connect(socket_fd, (struct sockaddr *)&server_address, sizeof server_address) == -1) {
        perror("connect");
        exit(1);
    }
 
    start_listen_in_new_thread(socket_fd);
 
while (1) {
        // Get user input
        printf("> ");
        fgets(message, BUFFER_SIZE, stdin);
 
        // Remove newline character from fgets input
        size_t len = strlen(message);
        if (len > 0 && message[len - 1] == '\n') {
            message[len - 1] = '\0';
        }
 
        // Send the message to the server
        send(socket_fd, message, strlen(message), 0);
        printf("Message sent to server\n");
 
        // If user types "exit", break the loop
        if (strcmp(message, "exit") == 0) {
            printf("Exiting...\n");
            break;
        }
    }
 
    close(socket_fd);
    return 0;
}
void start_listen_in_new_thread(int socket_fd) {
    pthread_t id;
    int *fd_ptr = malloc(sizeof(int));
    if (fd_ptr == NULL) {
        perror("Failed to allocate memory");
        exit(EXIT_FAILURE);
    }
    *fd_ptr = socket_fd;
 
    if (pthread_create(&id, NULL, listen_and_print, (void *)fd_ptr) != 0) {
        perror("Failed to create thread");
        free(fd_ptr);
        exit(EXIT_FAILURE);
    }
 
    pthread_detach(id); // Detach to avoid resource leaks
}