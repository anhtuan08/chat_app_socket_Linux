#define _POSIX_C_SOURCE 200809L
 
#include "common.h"
 
#define BUFFER_SIZE 1024
#define MAX_USER 10
 
int client_count = 0;
int server_socket_fd;
 
void error(const char *msg)
{
    perror(msg);
    exit(1);
}
 
 
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
 
typedef struct {
    int socket_fd;
    struct sockaddr_in address;
    int addr_len;
} client_t;
 
client_t *clients[MAX_USER];
 
void add_client(client_t *cl) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_USER; ++i) {
        if (clients[i] == NULL) {
            clients[i] = cl;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}
 
void remove_client(int socket_fd) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_USER; ++i) {
        if (clients[i] && clients[i]->socket_fd == socket_fd) {
            clients[i] = NULL;
            break;
        }
    }
    client_count--;
    pthread_mutex_unlock(&clients_mutex);
}
 
 
void broadcast_message(const char *message, int sender_fd) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_USER; ++i) {
        if (clients[i] && clients[i]->socket_fd != sender_fd) {
            send(clients[i]->socket_fd, message, strlen(message), 0);
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}
 
/* Thread routine to serve connection to client. */
void *handle_client (void *arg);
 
/* Signal handler to handle SIGTERM and SIGINT signals. */
void handler_signal(int signal);
 
// Broadcast a message to a specific client
void send_message_to_client(int client_socket, char *message) {
    send(client_socket, message, strlen(message), 0);
}
 
int main(int argc, char *argv[])
{
    int new_socket, PORT;
    pthread_t pthread;
 
    if (argc != 2) {
        printf("Usage: %s <PORT>\n", argv[0]);
        return 1; // Exit with error if port is not provided
    }
 
    // Convert the port number from string to integer
    PORT = atoi(argv[1]);
    if (PORT <= 0 || PORT > 65535) {
        printf("Invalid PORT number. Please provide a number between 1 and 65535.\n");
        return 1;
    }
 
    struct sockaddr_in server_address, client_address;
    socklen_t client_len = sizeof(client_address);
 
    //Create socket
    server_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket_fd < 0)
        error("Error while creating socket!");
 
    // Register the signal handler for SIGINT
    signal(SIGINT, handler_signal);
 
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(PORT);
 
 
    // Bind socket to the specified IP and port
    if (bind(server_socket_fd, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }
 
    // Listen for connections
    if (listen(server_socket_fd, MAX_USER) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }
 
    printf("Server is listening on %s at PORT: %d\n", inet_ntoa(server_address.sin_addr), PORT);
 
 
    // Allocate memory for thread arguments   
    while (1) {
    new_socket = accept(server_socket_fd, (SA*)&client_address, &client_len);
 
    if (new_socket < 0) {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }
        printf( "New client connected\n");
        client_count+=1;
    
    
        // Allocate memory for thread arguments
        client_t* client_data = (client_t*)malloc(sizeof(client_t));
        if (!client_data) {
            perror("Memory allocation failed");
            close(new_socket);
            continue;
        }
 
        // Initialize thread arguments
        client_data->socket_fd = new_socket;
        client_data->address = client_address;
        client_data->addr_len = client_len;
 
        add_client(client_data);
 
        printf("socket of client is %d\n", client_data->socket_fd);
 
        // Create a thread for the new client
        if (pthread_create(&pthread, NULL, handle_client, (void*)client_data) != 0) {
            perror("Thread creation failed");
            free(client_data);
            close(new_socket);
            continue;
        }
        printf("client count is %d\n", client_count);
 
        // Detach thread to allow automatic resource cleanup
        pthread_detach(pthread);
 
    }
 
    close(server_socket_fd);
    return 0;
}
 
void* handle_client(void* argc){
    client_t *cli = (client_t *)argc;
    char buffer[1024], temp[256];;
    
    // Communicate with the client
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytesRead = recv(cli->socket_fd, buffer, 1024, 0);
        if (bytesRead <= 0) {
            printf("Client Disconnected\n");
            close(cli->socket_fd);
            break;
        }
        else{
 
            printf("Received from client %d: %s\n", cli->socket_fd, buffer);
 
            // Example: Check for direct messaging command
            if (strncmp(buffer, "~send_to_", 9) == 0) {
                int target_fd = atoi(buffer + 9); // Extract target FD from message
                printf("%d\n", target_fd);
                
                snprintf(temp, BUFFER_SIZE, "Direct message from %d: %s", cli->socket_fd, buffer + 11);
                send_message_to_client(target_fd, temp);
            }
            else if (strncmp(buffer, "~list_", 6) == 0) {
                char list[BUFFER_SIZE] = "Connected clients:\n";
                pthread_mutex_lock(&clients_mutex);
                for (int i = 0; i < MAX_USER; ++i) {
                    if (clients[i]) {
                        char temp[50];
                        snprintf(temp, sizeof(temp), "Client ID: %d\n", clients[i]->socket_fd);
                        strcat(list, temp);
                    }
                }
                pthread_mutex_unlock(&clients_mutex);
 
                send(cli->socket_fd, list, strlen(list), 0);
            } else if (strncmp(buffer, "~id_", 4) == 0) {
                char id_msg[BUFFER_SIZE];
                snprintf(id_msg, BUFFER_SIZE, "Your client ID is: %d\n", cli->socket_fd);
                send(cli->socket_fd, id_msg, strlen(id_msg), 0);
            }
            else if(strncmp(buffer, "~br_", 4) == 0){
                snprintf(temp, BUFFER_SIZE, "Client %d says: %s", cli->socket_fd, buffer);
                broadcast_message(temp, cli->socket_fd);
            }
        }
    }
 
    close(cli->socket_fd);
    remove_client(cli->socket_fd);
    free(cli);
    return NULL;
    
}
 
void handler_signal(int signal){
    if (signal == SIGINT) {
        printf("\nSIGINT received. Shutting down the server...\n");
        // notify_clients_and_exit();
    }
}
 
 
 