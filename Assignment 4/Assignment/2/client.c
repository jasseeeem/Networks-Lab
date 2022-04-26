#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX 1024
#define PORT 8080
#define SA struct sockaddr_in

int main()
{

    int sockfd;
    struct sockaddr_in servaddr;
    char buff[MAX];

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        printf("Socket creation failed ...\n");
        exit(1);
    }
    // else
    //     printf("Socket created successfully ...\n");

    // initialize servadd with 0
    memset(&servaddr, 0, sizeof(servaddr));

    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // connect to the socket
    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0)
    {
        printf("Could not connect to the server\n");
        exit(1);
    }
    else
        printf("Connected to the server\n");

    while (1)
    {
        printf("enter the query: ");
        int n = 0;

        // write characters to buff until enter is encountered
        while ((buff[n++] = getchar()) != '\n')
            ;
        buff[n - 1] = '\0';
        write(sockfd, buff, MAX);
        if (strncmp(buff, "exit", 4) == 0)
        {
            close(sockfd);
            printf("Disconnected from server.\n");
            exit(0);
        }
        memset(buff, 0, MAX);
        if (read(sockfd, buff, MAX) < 0)
        {
            printf("Error in receiving data\n");
        }
        else
        {
            printf("\n[Server]: %s\n\n", buff);
        }
    }

    return 0;
}