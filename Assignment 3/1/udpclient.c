#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#define MAX 80
#define PORT 8080
#define SA struct sockaddr

int main()
{
    int sockfd;

    // sockaddr_in structure specifies a transport address and port
    struct sockaddr_in servaddr;

    /*  creating socket
        AF_INET - IPV4
        SOCK_DGRAM - UDP
        0 - IP Protocol     */
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1)
    {
        printf("Socket creation failed...\n");
        exit(1);
    }
    else
        printf("Socket successfully created...\n");

    // writing zeroes to servaddr
    bzero(&servaddr, sizeof(servaddr));

    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    // htons() swaps bytes so that numbers are stored in network byte order
    servaddr.sin_port = htons(PORT);

    // connect the client socket to server socket
    if (connect(sockfd, (SA *)&servaddr, sizeof(servaddr)) != 0)
    {
        printf("Could not connect to the server...");
        exit(1);
    }
    else
        printf("Connected to the server...");

    // chat
    char buff[MAX];
    int n, len;

    while (1)
    {
        bzero(buff, sizeof(buff));
        printf("\n\n>>> ");
        int nn = 0;

        // write characters to buff until enter is encountered
        while ((buff[nn++] = getchar()) != '\n')
            ;
        buff[nn] = '\0';

        // write MAX number of characters into the socket
        sendto(sockfd, buff, sizeof(buff), MSG_CONFIRM, (SA *)&servaddr, sizeof(servaddr));

        // write(sockfd, buff, sizeof(buff));
        bzero(buff, sizeof(buff));

        // reads MAX number of characters from the socket
        n = recvfrom(sockfd, (char *)buff, MAX, MSG_WAITALL, (SA *)&servaddr, &len);

        printf("From Server : %s", buff);
        if ((strncmp(buff, "exit", 4)) == 0)
        {
            printf("\nClient Exit...\n");
            break;
        }
    }

    // close the socket
    close(sockfd);
}
