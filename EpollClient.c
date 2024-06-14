// echo_client_epoll.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <errno.h>

#define PORT 12345
#define BUFFER_SIZE 1024
#define MAX_EVENTS 2

int main() {
    int sock = 0, valread, epfd;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};
    struct epoll_event ev, events[MAX_EVENTS];

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

    // Create an epoll instance
    epfd = epoll_create1(0);
    if (epfd == -1) {
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }

    // Add STDIN to epoll
    ev.events = EPOLLIN;
    ev.data.fd = STDIN_FILENO;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, STDIN_FILENO, &ev) == -1) {
        perror("epoll_ctl: stdin");
        exit(EXIT_FAILURE);
    }

    // Add the socket to epoll
    ev.events = EPOLLIN;
    ev.data.fd = sock;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, sock, &ev) == -1) {
        perror("epoll_ctl: sock");
        exit(EXIT_FAILURE);
    }

    while (1) {
        int nfds = epoll_wait(epfd, events, MAX_EVENTS, -1);
        if (nfds == -1) {
            perror("epoll_wait");
            exit(EXIT_FAILURE);
        }

        for (int n = 0; n < nfds; ++n) {
            if (events[n].data.fd == STDIN_FILENO) {
                valread = read(STDIN_FILENO, buffer, BUFFER_SIZE);
                if (valread > 0) {
                    buffer[valread] = '\0';
                    send(sock, buffer, valread, 0);
                }
            } else if (events[n].data.fd == sock) {
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
    }

    close(epfd);
    close(sock);
    return 0;
}
