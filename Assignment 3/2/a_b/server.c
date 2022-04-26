#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#define MAX 500
#define PORT 8080
#define SA struct sockaddr
#define MESSAGE_SIZE 500

void sendfile(FILE *fp, int sockfd)
{
    int n;
    char filePacket[MESSAGE_SIZE];
    memset(filePacket, 0, MESSAGE_SIZE);
    while ((n = fread(filePacket, sizeof(char), MESSAGE_SIZE, fp)) > 0)
    {
        if (n != MESSAGE_SIZE && ferror(fp))
        {
            perror("Read File Error");
            exit(1);
        }
        if (write(sockfd, filePacket, n) == -1)
        {
            perror("Cannot send the file");
            exit(1);
        }
        memset(filePacket, 0, MESSAGE_SIZE);
    }
    return;
}

void func(int connfd)
{
    char buff[MAX];
    char *filename = "send_50mb";

    // infinite loop for chat
    while (1)
    {
        read(connfd, buff, sizeof(buff));
        printf("\nFrom client : %s", buff);
        if (strncmp(buff, "Bye", 3) == 0)
        {
            printf("Server exit...\n");
            close(connfd);
            break;
        }
        else if (strncmp(buff, "GivemeyourVideo", 15) == 0)
        {
            FILE *fp = fopen(filename, "r");
            if (!fp)
            {
                perror("File Open");
                break;
            }
            else
            {
                struct stat st;
                char buffer[32];

                // get the file and store it in the buffer st
                stat(filename, &st);
                // store the file size in buff
                snprintf(buffer + strlen(buffer), sizeof buffer - strlen(buffer), "%ld", st.st_size);

                // send the file size
                write(connfd, buffer, MESSAGE_SIZE);
                printf("File Opened successfully\n");

                // send the actual file 500 bytes at a time
                sendfile(fp, connfd);
                printf("File sent successfully\n");
            }
        }
        else
        {
            strcpy(buff, "sduiafh");
            write(connfd, buff, sizeof(buff));
            printf("Meaningless term sent\n");
        }
        memset(&buff, 0, strlen(buff) + 1);
    }
}

// Driver function
int main()
{
    int sockfd, connfd, len;
    struct sockaddr_in servaddr, cli;

    // socket creation and verification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        printf("Socket creation failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully created..\n");
    bzero(&servaddr, sizeof(servaddr));

    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);

    // Binding newly created socket to given IP and verification
    if ((bind(sockfd, (SA *)&servaddr, sizeof(servaddr))) != 0)
    {
        printf("Socket binding failed...\n");
        exit(1);
    }
    else
        printf("Socket successfully binded..\n");

    // Now server is ready to listen and verification
    if ((listen(sockfd, 5)) != 0)
    {
        printf("Listening failed...\n");
        exit(0);
    }
    else
        printf("Server listening..\n");
    len = sizeof(cli);

    // Accept the data packet from client and verification
    connfd = accept(sockfd, (SA *)&cli, &len);
    if (connfd < 0)
    {
        printf("Server accepting failed...\n");
        exit(0);
    }
    else
        printf("Server accepted the client...\n");

    // Function for chatting between client and server
    func(connfd);

    // After chatting close the socket
    close(sockfd);
}
