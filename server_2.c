#define _POSIX_C_SOURCE 200809L

#include "common.h"

#define BUFFER_SIZE 1024
#define LISTEN_BACKLOG 10
#define MAX_USER 10

int client_count = 0;
int server_socket_fd;

int client[MAX_USER];

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

typedef struct pthread_arg_t {
    int id;
    int new_socket_fd;
    struct sockaddr_in client_address;
} pthread_arg_t;


/* Thread routine to serve connection to client. */
void *handle_client (void *arg);

/* Signal handler to handle SIGTERM and SIGINT signals. */
void handler_signal(int signal);

int list_free_user(const char* list);

void connected_between_client(int new_socket_fd, char buffer[], int client_id);

void notify_clients_and_exit();

void add_client();

const char* list = "list_free_user";

int main(int argc, char *argv[])
{
    int client_fd, new_socket, PORT;
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

    add_client();

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
    char buffer[1024] = {0};
    int new_socket_client = pthread_client->new_socket_fd;
    client[pthread_client->id - 1] = pthread_client->id;
    
    // Communicate with the client
    while (1) {
        memset(buffer, 0, sizeof(buffer));
        int bytesRead = read(new_socket_client, buffer, 1024);
        if (bytesRead <= 0) {
            printf("Client Disconnected\n");
            close(new_socket_client);
            break;
        }

        // if(!bcmp(buffer, "~connect_to_", 12)){
        //     connected_between_client(new_socket_client ,buffer, client[pthread_client->id - 1]);
        // }

        printf("Message from client with id%d: %s\n", client[pthread_client->id - 1], buffer);
        
        // Echo message back to the client
        send(new_socket_client, buffer, bytesRead, 0);
    }
    return NULL;
}

void connected_between_client(int new_socket_fd, char buffer[], int client_id){
    int to = ((int)(buffer[12] - '0')*10 + (int)(buffer[13] - '0'));
    if (client[to] == -1) {
    char temp[256] = "Server:user_not_exist";
    write(new_socket_fd, temp, 255);
    } else if (client[to] != 0) {
    char temp[256] = "Server:user_busy";
    write(new_socket_fd, temp, 255);
    } else if (to == client_id) {
    char temp[256] = "Server:connecting to self is not allowed";
    write(new_socket_fd, temp, 255);
    }
    else{
        printf("Connected betwwen client%d and client%d\n", client_id, to);
    }
}

void handler_signal(int signal){
    if (signal == SIGINT) {
        printf("\nSIGINT received. Shutting down the server...\n");
        notify_clients_and_exit();
    }
}

// Notify all clients and close their connections
void notify_clients_and_exit() {
    ///pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_USER; ++i) {
        if (client[i] != 0) {
            send(client[i], "Server shutting down. Goodbye!\n", 32, 0);
            close(client[i]);
            client[i] = 0;
        }
    }
    //pthread_mutex_unlock(&clients_mutex);

    // Close the server socket
    close(server_socket_fd);
    printf("\nServer shutdown complete.\n");
    exit(0);
}

void add_client(){
    for(int i = 0; i < MAX_USER; i++){
        client[i] = 0;
    }
}