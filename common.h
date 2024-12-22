#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h> 
#include <pthread.h>
#include <signal.h>


#define SA struct sockaddr


void err_n_die(const char* msg, ...);

void create_socket(int* sockfd);

struct sockaddr_in* server_address(const char* ip, uint16_t host);


#endif