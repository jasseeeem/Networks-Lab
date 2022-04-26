#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX 1024
#define MAX_CLIENTS 32
#define PORT 8080
#define SA struct sockaddr_in

void solve(char buff[MAX], int connfd)
{
    char *ptr, *ptr1, op;
    long op1, op2, result;
    int count = 0, op_count = 0;
    op1 = strtol(buff, &ptr, 10);
    while (ptr[count] == ' ' || ptr[count] == '+' || ptr[count] == '-' || ptr[count] == '*' || ptr[count] == '/')
    {
        if (ptr[count] != ' ')
        {
            op = ptr[count];
            op_count++;
        }
        count++;
    }
    if (op_count != 1)
    {
        strcpy(buff, "Invalid Expression");
        printf("[Server]: %s\n", buff);
        write(connfd, buff, strlen(buff));
        return;
    }
    op2 = strtol(ptr + count, &ptr1, 10);
    switch (op)
    {
    case '+':
        result = op1 + op2;
        break;
    case '-':
        result = op1 - op2;
        break;
    case '*':
        result = op1 * op2;
        break;
    case '/':
        if (op2 == 0)
        {
            strcpy(buff, "Cannot divide by 0");
            printf("[Server]: %s\n", buff);
            write(connfd, buff, strlen(buff));
            return;
        }
        result = op1 / op2;
        break;
    }
    sprintf(buff, "%li", result);
    printf("[Server]: %s\n", buff);
    write(connfd, buff, strlen(buff));
    return;
}

int get_client_num(int connfd, int num_clients, int clients[])
{
    while (num_clients--)
    {
        if (clients[num_clients] == connfd)
        {
            return num_clients + 1;
        }
    }
    return -1;
}

int main()
{

    int sockfd, connfd, clients[MAX_CLIENTS], num_clients = 0;
    socklen_t addr_size;
    char buff[MAX];
    struct sockaddr_in servaddr, cli;
    pid_t childpid;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        printf("Could not connect\n");
        exit(1);
    }
    // printf("Socket created successfully\n");

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        printf("Could not bind\n");
        exit(1);
    }
    // printf("[+]Bind to port %d\n", PORT);

    if (listen(sockfd, 10) == 0)
    {
        printf("Server Listening ...\n");
    }
    else
    {
        printf("Could not listen\n");
    }

    while (1)
    {
        connfd = accept(sockfd, (struct sockaddr *)&cli, &addr_size);
        if (connfd < 0)
        {
            exit(1);
        }
        clients[num_clients++] = connfd;
        printf("\nConnected with client %d\n\n", num_clients);

        if ((childpid = fork()) == 0)
        {
            close(sockfd);

            while (1)
            {
                if (read(connfd, buff, MAX) <= 0 || strncmp(buff, "exit", 4) == 0)
                {
                    printf("\nDisconnected from Client%d\n\n", get_client_num(connfd, num_clients, clients));
                    break;
                }
                else
                {
                    printf("[Client%d]: %s\n", get_client_num(connfd, num_clients, clients), buff);
                    solve(buff, connfd);
                    memset(buff, 0, sizeof(buff));
                }
            }
        }
    }

    close(connfd);

    return 0;
}