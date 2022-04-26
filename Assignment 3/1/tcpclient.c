#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#define MAX 80
#define PORT 8080
#define SA struct sockaddr
void func(int sockfd)
{
    char buff[MAX];
    int n;
    while (1)
    {
        bzero(buff, sizeof(buff));
        printf("\n\n>>> ");
        n = 0;

        // write characters to buff until enter is encountered
        while ((buff[n++] = getchar()) != '\n')
            ;

        // write MAX number of characters into the socket
        write(sockfd, buff, sizeof(buff));
        bzero(buff, sizeof(buff));

        // reads MAX number of characters from the socket
        read(sockfd, buff, sizeof(buff));
        printf("From Server : %s", buff);
        if ((strncmp(buff, "exit", 4)) == 0)
        {
            printf("\n\nClient Exit...\n");
            break;
        }
    }
}

int main()
{
    int sockfd, connfd;

    // sockaddr_in structure specifies a transport address and port
    struct sockaddr_in servaddr, cli;

    /*  creating socket
        AF_INET - IPV4
        SOCK_STREAM - TCP
        0 - IP Protocol     */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        printf("Socket creation failed...\n");
        exit(1);
    }
    else
        printf("Socket successfully created..\n");

    // writing zeroes to servaddr
    bzero(&servaddr, sizeof(servaddr));

    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    // htons() swaps bytes so that numbers are stored in network byte order
    servaddr.sin_port = htons(PORT);

    // connect the client socket to server socket
    if (connect(sockfd, (SA *)&servaddr, sizeof(servaddr)) != 0)
    {
        printf("Could not connect to the server...");
        exit(1);
    }
    else
        printf("Connected to the server..");

    // function for chat
    func(sockfd);

    // close the socket
    close(sockfd);
}
