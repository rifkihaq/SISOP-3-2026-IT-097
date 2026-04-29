#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include "protocol.h"

Client clients[MAX_CLIENTS];
int client_count = 0;

void log_event(const char *type, const char *msg) {
    FILE *f = fopen("history.log", "a");
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);

    fprintf(f, "[%04d-%02d-%02d %02d:%02d:%02d] [%s] %s\n",
        tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday,
        tm->tm_hour, tm->tm_min, tm->tm_sec,
        type, msg);

    fclose(f);
}

void broadcast(char *msg, int sender) {
    for(int i = 0; i < client_count; i++) {
        if(clients[i].socket != sender) {
            send(clients[i].socket, msg, strlen(msg), 0);
        }
    }
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr*)&address, sizeof(address));
    listen(server_fd, 5);

    printf("SERVER ONLINE\n");
    log_event("System", "[SERVER ONLINE]");

    while(1) {
        new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);

        char name[50];
        read(new_socket, name, sizeof(name));

        // cek nama unik
        int exists = 0;
        for(int i=0;i<client_count;i++){
            if(strcmp(clients[i].name, name)==0){
                exists = 1;
                break;
            }
        }

        if(exists){
            char *msg = "Name already used\n";
            send(new_socket, msg, strlen(msg), 0);
            close(new_socket);
            continue;
        }

        strcpy(clients[client_count].name, name);
        clients[client_count].socket = new_socket;
        client_count++;

        char logmsg[100];
        sprintf(logmsg, "User '%s' connected", name);
        log_event("System", logmsg);

        if(fork() == 0) {
            char buffer[BUFFER_SIZE];

            while(1) {
                int valread = read(new_socket, buffer, BUFFER_SIZE);

                if(valread <= 0) break;

                buffer[valread] = '\0';

                if(strncmp(buffer, "/exit", 5)==0){
                    break;
                }

                broadcast(buffer, new_socket);
                log_event("User", buffer);
            }

            close(new_socket);
            exit(0);
        }
    }

    return 0;
}
