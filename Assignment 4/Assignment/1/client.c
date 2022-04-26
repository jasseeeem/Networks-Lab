#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <fcntl.h>

#define MAX 1024
#define PORT 8080
#define SA struct sockaddr_in

int main()
{
    // structure for storing time
    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 100;

    // file descriptor set for the select function
    fd_set sets;

    // initialize file descriptor set to contain no file descriptors
    FD_ZERO(&sets);

    int sockfd, connfd, n = 0, len;
    char username[16] = " ";
    struct sockaddr_in servaddr, cli;
    char buff[MAX], buff1[MAX];

    // TCP socket creation and verification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        printf("Socket creation failed ...\n");
        exit(1);
    }
    else
        printf("Socket created successfully ...\n");

    // initialize servadd with 0
    memset(&servaddr, 0, sizeof(servaddr));

    // assign IP, PORT
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);

    printf("\n====== WELCOME TO THE CHAT ROOM ======\n\n");
    printf("Enter your username: ");
    scanf("%s", username);
    username[strlen(username)] = '\0';

    // connect to the socket
    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0)
    {
        printf("\nCould not connect to the server ...\n");
        exit(1);
    }
    else
        printf("\nConnected to server successfully ...\n");

    // write the username into the socket
    n = write(sockfd, username, strlen(username));
    if (n < 0)
    {
        perror("Could not write to the socket");
        exit(1);
    }

    // fcntl - manipulate file descriptor
    // 0 - stdin
    // F_GETFL - return the file access mode and the file status flags
    int flags = fcntl(0, F_GETFL);
    // set the file status flags to flags
    fcntl(0, F_SETFL, flags | O_NONBLOCK);

    while (1)
    {
        memset(buff, 0, sizeof(buff));
        memset(buff1, 0, sizeof(buff1));
        fgets(buff, MAX, stdin);
        // replace \n with 0
        buff[strcspn(buff, "\n")] = 0;

        // write the message into the socket
        if (write(sockfd, buff, strlen(buff)) < 0)
        {
            perror("Could not write");
            exit(1);
        }
        if (strncmp("bye", buff, 3) == 0)
        {
            printf("You left the chat\n");
            break;
        }
        memset(buff, 0, sizeof(buff));

        // FD_SET(fd, &fdset) - sets the bit for the file descriptor fd in the file descriptor set fdset
        FD_SET(sockfd, &sets);

        // select() allows a program to monitor multiple file descriptors
        // n - number of file descriptors that triggers the event
        // nfds - highest-numbered file descriptors in any of the 3 sets + 1
        n = select(sockfd + 1, &sets, NULL, NULL, &timeout);

        // timed out without any message
        if (n == 0)
            continue;
        // if one of the sockets change, read from the socket
        else
        {
            n = read(sockfd, buff1, sizeof(buff1));
            if (n == 0)
            {
                printf("Server hung up ...\n");
                break;
            }
            len = strlen(buff1);
            if (buff1[len - 1] == '\n')
                buff1[len - 1] = 0;
        }
        if (n < 0)
        {
            perror("Error reading other messages");
            exit(1);
        }
        // one of the client sent bye
        if (strstr(buff1, "bye") != NULL)
        {
            // a - part of buff1 after :
            char *a = strchr(buff1, ':');
            // finding username length
            int b = (a == NULL ? -1 : a - buff1);
            // printing first b characters
            printf("%.*s", b, buff1);
            printf(" left the chat \n");
            continue;
        }
        if (!(strcmp(buff1, " ") && buff1[0] == '\0'))
            printf("%s\n", buff1);
    }

    close(sockfd);
}