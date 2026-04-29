#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "protocol.h"

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};

    sock = socket(AF_INET, SOCK_STREAM, 0);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

    connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

    char name[50];
    printf("Enter your name: ");
    scanf("%s", name);

    send(sock, name, strlen(name), 0);

    // thread sederhana pakai fork
    if(fork()==0){
        while(1){
            char msg[BUFFER_SIZE];
            fgets(msg, BUFFER_SIZE, stdin);
            send(sock, msg, strlen(msg), 0);
        }
    } else {
        while(1){
            int valread = read(sock, buffer, BUFFER_SIZE);
            buffer[valread] = '\0';
            printf("%s", buffer);
        }
    }

    return 0;
}
