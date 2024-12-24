#define _POSIX_C_SOURCE 200809L

#include "common.h"

#define BUFFER_SIZE 1024
#define LISTEN_BACKLOG 10
#define MAX_USER 10

#define TEMP_SIZE 256
int client_count = 0;
int server_socket_fd;

pthread_mutex_t clients_mutex;

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

typedef struct{
    int id;
    int socket;
} Client;

Client clients[MAX_USER];

typedef struct pthread_arg_t {
    int id;
    int new_socket_fd;
    struct sockaddr_in client_address;
} pthread_arg_t;


/* Thread routine to serve connection to client. */
void *handle_client (void *arg);

/* Signal handler to handle SIGTERM and SIGINT signals. */
void handler_signal(int signal);

int find_client_socket_by_id(int id_client) {
    pthread_mutex_lock(&clients_mutex);
    int socket = 0;
    for (int i = 0; i < MAX_USER; ++i) {
        if (clients[i].id == id_client) {
            clients[i].socket = 1;
            socket = clients[i].socket;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
    return socket;
}

// Broadcast a message to a specific client
void send_message_to_client(int client_socket, char *message) {
    send(client_socket, message, strlen(message), 0);
}

void notify_clients_and_exit();

void connect_betwwen_2_client(int source, int target);

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
    
    
        // Allocate memory for thread arguments
        pthread_arg_t* client_data = (pthread_arg_t*)malloc(sizeof(pthread_arg_t));
        if (!client_data) {
            perror("Memory allocation failed");
            close(new_socket);
            continue;
        }

        // Initialize thread arguments
        client_data->id = ++client_count;
        client_data->new_socket_fd = new_socket;
        client_data->client_address = client_address;

        // Create a thread for the new client
        if (pthread_create(&pthread, NULL, handle_client, (void*)client_data) != 0) {
            perror("Thread creation failed");
            free(client_data);
            close(new_socket);
            continue;
        }

        // Detach thread to allow automatic resource cleanup
        pthread_detach(pthread);

    }

    close(server_socket_fd);
    return 0;
}

void* handle_client(void* argc){
    pthread_arg_t *pthread_client = (pthread_arg_t* )argc;
    char buffer[1024], temp[256];;
    int new_socket_client = pthread_client->new_socket_fd;
    clients[pthread_client->id].id = pthread_client->id;
    
    // Communicate with the client
    while (1) {
        memset(buffer, 0, sizeof(buffer));
        int bytesRead = read(new_socket_client, buffer, 1024);
        if (bytesRead <= 0) {
            printf("Client Disconnected\n");
            close(new_socket_client);
            break;
        }

            int connected_to = 0;
            memset(temp, 0, sizeof(temp));
         if (strncmp(buffer, "~connect_to_", 12) == 0) {
            int target_id = ((int)(buffer[12] - '0'));
            connected_to = find_client_socket_by_id(target_id);

            if (connected_to) {
                snprintf(temp, TEMP_SIZE, "Connected to Client %d.\n", target_id);
                send_message_to_client(new_socket_client, temp);
                connect_betwwen_2_client(clients[pthread_arg_t->id].id, target_id);

                printf("case2\n");
            } else {
                snprintf(temp, TEMP_SIZE, "Client %d not found.\n", target_id);
                send_message_to_client(new_socket_client, temp);
                printf("case3\n");
            }
         }

        printf("Message from client: %d with id%d: %s\n", clients[pthread_client->id].id, clients[pthread_client->id].id , buffer);
        
        // Echo message back to the client
        send(new_socket_client, buffer, bytesRead, 0);
    }
    return NULL;
}

void handler_signal(int signal){
    if (signal == SIGINT) {
        printf("\nSIGINT received. Shutting down the server...\n");
        notify_clients_and_exit();
    }
}

void connect_betwwen_2_client(int source, int target){
    pritnf("Seasions betwwen two clients %d with %dd\n", source,);

}

// Notify all clients and close their connections
void notify_clients_and_exit() {
    ///pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_USER; ++i) {
        if (clients[i].id != 0) {
            send(clients[i].socket, "Server shutting down. Goodbye!\n", 32, 0);
            close(clients[i].socket);
            clients[i].id = 0;
        }
    }
    //pthread_mutex_unlock(&clients_mutex);

    // Close the server socket
    close(server_socket_fd);
    printf("\nServer shutdown complete.\n");
    exit(0);
}
