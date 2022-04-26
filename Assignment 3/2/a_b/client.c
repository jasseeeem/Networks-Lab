#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/time.h>
#define MAX 500
#define PORT 8080
#define SA struct sockaddr_in
#define MESSAGE_SIZE 500

void add_data(FILE *fd, float time_slice, ssize_t bytes)
{
    fprintf(fd, "%f %ld\n", time_slice, bytes);
}

void writefile(int sockfd)
{
    char buff[MAX];

    // file where the received data should be stored in
    FILE *fp = fopen("receive_50mb", "w");
    // file where the transmission data is stored
    FILE *fd;
    fd = fopen("transmission_speed.dat", "w");
    ssize_t total = 0, prev_total = 0;

    // read the first 500 bytes
    int num_bytes = read(sockfd, &buff, MESSAGE_SIZE);
    if (num_bytes == -1)
    {
        perror("Read Error");
        exit(1);
    }
    printf("Message size: %d\n", num_bytes);
    printf("Total file size: %s\n", buff);
    long long int x = atoi(buff);

    // set initial time as the time when reading file is started

    struct timeval prev_time;
    gettimeofday(&prev_time, NULL);
    float time_slice = 0;
    // reading 500 bytes at a time
    while (total <= x)
    {
        struct timeval curr_time, sub_time;
        gettimeofday(&curr_time, NULL);
        timersub(&curr_time, &prev_time, &sub_time);
        if (sub_time.tv_sec >= 0.1)
        {
            time_slice += 0.1;
            add_data(fd, time_slice, total - prev_total);
            prev_time = curr_time;
            prev_total = total;
        }
        total += num_bytes;
        if (num_bytes < 2)
            break;

        // write the 500 bytes of data into the file
        if (fwrite(buff, sizeof(char), num_bytes, fp) != num_bytes)
        {
            perror("Write File Error");
            exit(1);
        }

        // number of bytes read is more than than the file size
        if (total >= x)
            break;
        memset(buff, 0, sizeof(buff));

        // read next 500 bytes
        num_bytes = read(sockfd, buff, MESSAGE_SIZE);
        if (num_bytes == -1)
        {
            perror("Read File Error");
            exit(1);
        }
    }
    fclose(fd);
    printf("File received successfully\nTransmission speed recorded");
    return;
}

int main()
{
    int sockfd, connfd;
    struct sockaddr_in servaddr, cli;
    char buff1[MAX], buff2[MAX];
    int n, len;
    // socket creation and varification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        printf("socket creation failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully created..\n");
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);

    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0)
    {
        printf("connection with the server failed...\n");
        exit(0);
    }
    else
        printf("connected to the server");
    bzero(buff1, sizeof(buff1));
    bzero(buff2, sizeof(buff2));
    char *filename = "read";
    while (1)
    {
        memset(buff1, 0, sizeof(buff1));
        memset(buff2, 0, sizeof(buff2));
        printf("\n\n>>> ");
        fgets(buff1, 500, stdin);
        if (strncmp(buff1, "Bye", 3) == 0)
        {
            write(sockfd, buff1, strlen(buff1));
            printf("Client exit...\n");
            exit(0);
        }
        else
        {
            write(sockfd, buff1, sizeof(buff1));
            if (strncmp(buff1, "GivemeyourVideo", 15) == 0)
                writefile(sockfd);
            else
            {
                read(sockfd, &buff2, 500);
                printf("%s", buff2);
                memset(&buff2, 0, sizeof(buff2));
            }
        }
    }
    close(sockfd);
}