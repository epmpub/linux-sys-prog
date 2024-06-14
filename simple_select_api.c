#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/select.h>
   #include <string.h>
#include <arpa/inet.h>
#include <bits/socket.h>



// Function to write data to a file
void write_to_file(const char *filename, const char *data) {
    int file = open(filename, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (file == -1) {
        perror("open output file");
        exit(EXIT_FAILURE);
    }
    if (write(file, data, strlen(data)) == -1) {
        perror("write");
        close(file);
        exit(EXIT_FAILURE);
    }
    close(file);
}


int main()
{
    fd_set readfds;
    struct timeval timeout;
    int ret;

    fcntl(STDIN_FILENO,F_SETFL,O_NONBLOCK);


    int client_fd;
    
    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }


    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(123456);


    int ret = connect(client_fd,( struct sockaddr* )&address,sizeof( struct sockaddr_in  ));

    while (1)
    {
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO,&readfds);
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;

        ret = select(STDIN_FILENO+1,&readfds,NULL,NULL,&timeout);
        if (ret == -1)
        {
            perror("select");
            exit(EXIT_FAILURE);
        }else if (ret == 0)
        {
            printf("Timeout occurred! No data after 5 seconds.\n");
        }else
        {
            if(FD_ISSET(STDIN_FILENO,&readfds))
            {
                printf("data is ready to read\n");
            }
            char buffer[1024];
            int bytes = read(STDIN_FILENO,buffer,sizeof(buffer)-1);
            if (bytes > 0)
            {
                buffer[bytes] = '\0';
                write_to_file("log.txt",buffer);
            }
            
        }

    }

    return 0;
}