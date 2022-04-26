#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#define MAX 80
#define PORT 8080
#define SA struct sockaddr
#define INITIAL_QUANTITY 10

struct fruit
{
    char name[10];
    int quantity;
};

// Driver function
int main()
{
    int sockfd, len, n;
    struct sockaddr_in servaddr, cli;

    struct fruit fr[5];

    strcpy(fr[0].name, "apple");
    fr[0].quantity = INITIAL_QUANTITY;
    strcpy(fr[1].name, "mango");
    fr[1].quantity = INITIAL_QUANTITY;
    strcpy(fr[2].name, "banana");
    fr[2].quantity = INITIAL_QUANTITY;
    strcpy(fr[3].name, "chikoo");
    fr[3].quantity = INITIAL_QUANTITY;
    strcpy(fr[4].name, "papaya");
    fr[4].quantity = INITIAL_QUANTITY;

    // socket creation and verification
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1)
    {
        printf("Socket creation failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully created..\n");
    bzero(&servaddr, sizeof(servaddr));
    bzero(&cli, sizeof(cli));

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
        printf("Socket successfully binded..\nServer listening...\n");

    len = sizeof(cli);
    char buff[MAX];

    // Chat
    while (1)
    {
        bzero(buff, MAX);
        n = recvfrom(sockfd, buff, MAX, MSG_WAITALL, (SA *)&cli, &len);
        printf("\nFrom client: %s", buff);

        if (strncmp("fruits", buff, 6) == 0)
        {
            strcpy(buff, "Enter the name of the fruit");
        }
        else if (strncmp("SendInventory", buff, 13) == 0)
        {
            strcpy(buff, "\n---Inventory---");
            // loop through all the structures and store the details in the buffer
            for (int i = 0; i < 5; i++)
            {
                snprintf(buff + strlen(buff), sizeof buff - strlen(buff), "\n%s: %d", fr[i].name, fr[i].quantity);
            }
        }
        else if (strncmp("exit", buff, 4) == 0)
        {
            printf("\nServer Exit...\n");
            strcpy(buff, "exit");
            sendto(sockfd, buff, sizeof(buff), MSG_CONFIRM, (SA *)&cli, len);
            break;
        }
        else
        {
            char fruit_name[MAX], quantity[MAX];
            char *token = strtok(buff, " ");
            strcpy(fruit_name, token);
            token = strtok(NULL, " ");
            strcpy(quantity, token);
            for (int i = 0; i < MAX; i++)
            {
                if (quantity[i] == '\n')
                {
                    quantity[i] = ' ';
                    break;
                }
            }
            bzero(buff, MAX);
            int quantity_num = atoi(quantity);
            for (int i = 0; i < 5; i++)
            {
                if (strcmp(fr[i].name, fruit_name) == 0)
                {
                    if (fr[i].quantity < quantity_num || quantity_num < 0)
                    {
                        strcpy(buff, "Not available");
                    }
                    else
                    {
                        fr[i].quantity -= quantity_num;
                        strcat(buff, "Removed ");
                        strcat(buff, quantity);
                        strcat(buff, fruit_name);
                        strcat(buff, "s from the stock");
                    }
                    break;
                }
            }
        }
        sendto(sockfd, buff, sizeof(buff), MSG_CONFIRM, (SA *)&cli, len);
    }

    // After chatting close the socket
    close(sockfd);
}
