// echo_client_poll.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <poll.h>

#define PORT 12345
#define BUFFER_SIZE 1024

int main() {
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};
    struct pollfd fds[2];

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection Failed");
        return -1;
    }

    printf("Connected to server. Type messages and press enter to send.\n");

    fds[0].fd = STDIN_FILENO;
    fds[0].events = POLLIN;
    fds[1].fd = sock;
    fds[1].events = POLLIN;

    while (1) {
        int ret = poll(fds, 2, -1); // -1 means to wait indefinitely

        if (ret < 0) {
            perror("poll error");
            break;
        }

        // Check for keyboard input
        if (fds[0].revents & POLLIN) {
            valread = read(STDIN_FILENO, buffer, BUFFER_SIZE);
            if (valread > 0) {
                buffer[valread] = '\0';
                send(sock, buffer, valread, 0);
            }
        }

        // Check for server response
        if (fds[1].revents & POLLIN) {
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
    }

    close(sock);
    return 0;
}
