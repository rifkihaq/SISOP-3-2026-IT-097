#ifndef PROTOCOL_H
#define PROTOCOL_H

#define PORT 8080
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

typedef struct {
    int socket;
    char name[50];
} Client;

#endif
