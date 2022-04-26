#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/select.h>

#define MAX 1024
#define MAX_CLIENTS 16
#define PORT 8080
#define SA struct sockaddr

// Driver function
int main()
{
    int sockfd, connfd, len, n, nbytes, yes = 1, fdmax; // max file desc number.
    char buff[MAX], username[16], message[MAX], usernames[MAX_CLIENTS][16];
    struct sockaddr_in servaddr, cli;

    // master file descriptor set
    fd_set master;
    // temporary file descriptor set
    fd_set read_fds;

    // initialize file descriptor set to contain no file descriptors
    FD_ZERO(&master);
    FD_ZERO(&read_fds);
    memset(usernames, 0, (sizeof *usernames) * 16);

    // socket creation and verification
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
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);

    // setsockopt - provides a program with the means to control socket behavior
    // SOL_SOCKET - socket layer itself
    // SO_REUSEADDR - allows a socket to forcibly bind to a port in use by another socke
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
    {
        perror("Could not control socket behavior ...");
        exit(1);
    }
    printf("Successfully controlling socket behavior ...\n");

    // Binding newly created socket to given IP and verification
    if ((bind(sockfd, (SA *)&servaddr, sizeof(servaddr))) != 0)
    {
        printf("Could not bind socket ...\n");
        exit(1);
    }
    else
        printf("Socket binded successfully ...\n");

    // FD_SET(fd, &fdset) - sets the bit for the file descriptor fd in the file descriptor set fdset
    FD_SET(sockfd, &master);

    // Listen to upto 50 clients
    if ((listen(sockfd, 50)) != 0)
    {
        printf("Listening failed ...\n");
        exit(1);
    }
    else
        printf("Server listening ...\n\n");

    // biggest file descriptor is fdmax
    fdmax = sockfd;

    memset(usernames, 0, sizeof(usernames));

    while (1)
    {
        read_fds = master;

        // FD_SETSIZE - maximum number of file descriptors that fd_set can hold
        if (select(FD_SETSIZE + 1, &read_fds, NULL, NULL, NULL) == -1)
        {
            perror("");
            exit(1);
        }
        // loop through all possible file descriptors
        for (int i = 0; i < FD_SETSIZE; i++)
        {
            // check if ith bit is set in read_fds
            if (FD_ISSET(i, &read_fds))
            {
                // handle new connection
                if (i == sockfd)
                {
                    len = sizeof(cli);
                    // accept the client
                    if ((connfd = accept(sockfd, (SA *)&cli, &len)) == -1)
                    {
                        perror("Could not accept the client ...\n");
                    }
                    else
                    {
                        // add the current connfd to master set
                        FD_SET(connfd, &master);
                        if (connfd > fdmax)
                        {
                            fdmax = connfd;
                        }
                        // receive the username of the client
                        memset(username, 0, sizeof(username));
                        memset(usernames[connfd], 0, sizeof(usernames[connfd]));
                        read(connfd, usernames[connfd], sizeof(usernames[connfd]));
                        strcpy(username, usernames[connfd]);
                        memset(message, 0, sizeof(message));
                        strcpy(message, username);
                        strcat(message, " joined the chat");
                        printf("New connection from %s on socket %d\n", username, connfd);

                        // list of connected users
                        char connected_usernames[255];
                        int users_exists = 0;
                        memset(connected_usernames, 0, sizeof(connected_usernames));
                        strcpy(connected_usernames, "Users online: \n");
                        for (int k = 0; k <= fdmax; k++)
                        {
                            // if kth bit is set and the socket is not the server and not the most recent connected user
                            if (FD_ISSET(k, &master) && k != sockfd && k != connfd)
                            {
                                users_exists = 1;
                                strcat(connected_usernames, usernames[k]);
                                strcat(connected_usernames, "\n");
                            }
                        }
                        // write only if there are connected users
                        if (users_exists == 1)
                        {
                            strcat(connected_usernames, "\n");
                            write(connfd, connected_usernames, sizeof(connected_usernames));
                        }
                        for (int k = 0; k <= FD_SETSIZE; k++)
                        {
                            if (FD_ISSET(k, &master))
                            {
                                if (k != sockfd && connfd != k)
                                {
                                    write(k, message, sizeof(message));
                                    if (strncmp("bye", buff, 3) == 0)
                                    {
                                        printf("Server closed ...\n");
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
                // handle message
                else
                {
                    memset(buff, 0, MAX);
                    if ((nbytes = read(i, buff, sizeof(buff))) <= 0)
                    {
                        if (nbytes == 0)
                            printf("%s: Socket %d closed\n", username, i);
                        else
                            perror("read error");
                        // close the socket
                        close(i);
                        // remove from the master set
                        FD_CLR(i, &master);
                    }
                    else
                    {
                        // send the message to all clients
                        memset(message, 0, sizeof(message));
                        strcpy(message, usernames[i]);
                        strcat(message, ": ");
                        strcat(message, buff);
                        if (strncmp("bye", buff, 3) == 0)
                        {
                            write(i, message, sizeof(message));
                            printf("Connection closed by %s\n", usernames[i]);
                        }
                        else
                            printf("%s: %s\n", usernames[i], buff);
                        // send message to all clients
                        for (int j = 0; j <= FD_SETSIZE; j++)
                        {
                            if (FD_ISSET(j, &master))
                            {
                                // if socket is not server socket and the sender's socket
                                if (j != sockfd && j != i)
                                {
                                    write(j, message, sizeof(message));
                                }
                            }
                        }
                        if (strncmp("bye", buff, 3) == 0)
                        {
                            // remove the socket from master
                            FD_CLR(i, &master);
                        }
                    }
                    memset(buff, 0, sizeof(buff));
                    memset(message, 0, sizeof(message));
                }
            }
        }
    }
    close(sockfd);
}
