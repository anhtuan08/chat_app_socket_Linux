#include "common.h"

const char* localHost_ip = "127.0.0.1";

void err_n_die(const char* msg, ...){
    perror(msg);
    exit(1);
}

void create_socket(int* sockfd){
    *sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0)
        err_n_die("Error while creating socket!\n");
    else
        printf("Socket created!\n");
}

struct sockaddr_in* server_address(const char* ip, uint16_t host){
    struct sockaddr_in *ser_add = malloc(sizeof(struct sockaddr_in));
    ser_add->sin_family = AF_INET;
    ser_add->sin_port = htons(host);    

    if(inet_pton(AF_INET, ip, &ser_add->sin_addr.s_addr) <= 0)
        err_n_die("Error while converting IP address!\n");
    else{
        printf("IP address converted!\n");
    }

    return ser_add;
}