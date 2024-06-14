// echo_client.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <errno.h>

#define PORT 12345
#define BUFFER_SIZE 1024

int main() {
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};
    fd_set readfds;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection Failed");
        return -1;
    }

    printf("Connected to server. Type messages and press enter to send.\n");

    while (1) {
        FD_ZERO(&readfds);

        // Add STDIN to set for keyboard input
        FD_SET(STDIN_FILENO, &readfds);

        // Add socket to set for server input
        FD_SET(sock, &readfds);

        int max_sd = sock > STDIN_FILENO ? sock : STDIN_FILENO;

        int activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

        if ((activity < 0) && (errno != EINTR)) {
            perror("select error");
        }

        // If something happened on the socket, then it's data from the server
        if (FD_ISSET(sock, &readfds)) {
            valread = read(sock, buffer, BUFFER_SIZE);
            if (valread == 0) {
                printf("Server disconnected\n");
                close(sock);
                exit(0);
            } else {
                buffer[valread] = '\0';
                printf("Echo from server: %s\n", buffer);
            }
        }

        // If something happened on STDIN, then it's keyboard input
        if (FD_ISSET(STDIN_FILENO, &readfds)) {
            valread = read(STDIN_FILENO, buffer, BUFFER_SIZE);
            if (valread > 0) {
                buffer[valread] = '\0';
                send(sock, buffer, valread, 0);
            }
        }
    }

    close(sock);
    return 0;
}
