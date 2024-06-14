// echo_server_epoll.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <errno.h>

#define PORT 12345
#define MAX_EVENTS 1024
#define BUFFER_SIZE 1024

int main() {
    int server_fd, new_socket, epoll_fd;
    struct sockaddr_in address;
    struct epoll_event ev, events[MAX_EVENTS];
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE];
    
    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Set socket options
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Bind the socket to the network address and port
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_fd, 3) < 0) {
        perror("listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Create epoll instance
    epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        perror("epoll_create1");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Add server_fd to epoll instance
    ev.events = EPOLLIN;
    ev.data.fd = server_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev) == -1) {
        perror("epoll_ctl: server_fd");
        close(server_fd);
        close(epoll_fd);
        exit(EXIT_FAILURE);
    }

    while (1) {
        int num_fds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (num_fds == -1) {
            perror("epoll_wait");
            break;
        }

        for (int i = 0; i < num_fds; ++i) {
            if (events[i].data.fd == server_fd) {
                // Accept new connections
                new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
                if (new_socket == -1) {
                    perror("accept");
                    continue;
                }

                // Add new socket to epoll instance
                ev.events = EPOLLIN | EPOLLET; // Edge-triggered
                ev.data.fd = new_socket;
                if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, new_socket, &ev) == -1) {
                    perror("epoll_ctl: new_socket");
                    close(new_socket);
                }

                printf("New connection, socket fd is %d, ip is : %s, port : %d\n",
                       new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));

            } else {
                // Handle read from connected client
                int client_fd = events[i].data.fd;
                int bytes_read = read(client_fd, buffer, BUFFER_SIZE);
                if (bytes_read == 0) {
                    // Client disconnected
                    printf("Client disconnected, socket fd is %d\n", client_fd);
                    close(client_fd);
                } else if (bytes_read < 0) {
                    perror("read error");
                    close(client_fd);
                } else {
                    buffer[bytes_read] = '\0';
                    printf("Received message: %s\n", buffer);
                    send(client_fd, buffer, bytes_read, 0);
                }
            }
        }
    }

    close(server_fd);
    close(epoll_fd);
    return 0;
}
