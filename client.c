//Multi client connet to server
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>

//Get server name from command line - Ip address

//Get server port from cmd line

//Get server host from server name

//Initialise Ipv4 server address with host

//Create TCp socket 

//Connect socket with server address

//Loop to send and recv 


#define SERVER_NAME_LEN_MAX 255
#define BUFFER_SIZE 1024


int main(int argc, char *argv[]) {
    char server_name[SERVER_NAME_LEN_MAX + 1] = { 0 };
    int socket_fd;
    struct hostent *server_host;
    struct sockaddr_in server_address;
    char buffer[1024];
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

    // /* Get server host from server name. */
    // server_host = gethostbyname(server_name);

    // /* Initialise IPv4 server address with server host. */
    // memset(&server_address, 0, sizeof server_address);
    // server_address.sin_family = AF_INET;
    // server_address.sin_port = htons(server_port);
    // server_address.sin_addr.s_addr = inet_pton()
    //memcpy(&server_address.sin_addr.s_addr, server_host->h_addr, server_host->h_length);

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

        if(trncmp(buffer, "~connect_to_", 12) == 0){
            check_id()
        }
        // Read server response
        memset(buffer, 0, BUFFER_SIZE);
        read(socket_fd, buffer, BUFFER_SIZE);
        printf("Message from server: %s\n", buffer);
    }

    close(socket_fd);
    return 0;
}