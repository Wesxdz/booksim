#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define SERVER "127.0.0.1"
#define PORT 12345
#define MAXBUF 1024

int main() {
    int sock;
    struct sockaddr_in server;
    char message[MAXBUF] , server_reply[MAXBUF];

    //Create socket
    sock = socket(AF_INET , SOCK_STREAM , 0);
    if (sock == -1) {
        printf("Could not create socket");
    }

    server.sin_addr.s_addr = inet_addr(SERVER);
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);

    //Connect to remote server
    if (connect(sock, (struct sockaddr *)&server , sizeof(server)) < 0) {
        perror("connect failed. Error");
        return 1;
    }

    printf("Connected\n");

    //Send some data
    printf("Enter message: ");
    fgets(message, sizeof(message), stdin);

    if(send(sock , message , strlen(message) , 0) < 0) {
        perror("Send failed");
        return 1;
    }

    //Receive a reply from the server
    if(recv(sock , server_reply , MAXBUF , 0) < 0) {
        perror("recv failed");
        return 1;
    }

    printf("Server reply: %s", server_reply);

    close(sock);
    return 0;
}
