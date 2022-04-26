#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>

#define PORT 8080
#define MAX 100
#define SERVER_IP "127.0.0.1"
#define MAX_FRAMES 30

typedef enum
{
    DATA,
    ACK
} FRAMEKIND;

struct MSG
{
    char data[MAX];
};

struct timeval timeout;

struct Frame
{
    FRAMEKIND type;
    unsigned int len;
    int seq;
    char *msg;
};

int main()
{
    int sockfd;
    struct sockaddr_in servaddr;
    int n, e, i, j, p = 0, ack, next_seq = 0, m;
    char buff1[MAX], buff2[MAX], buff3[MAX], q, w;
    char del = '$';
    struct Frame *frame = (struct Frame *)malloc(MAX_FRAMES * sizeof(struct Frame));

    // socket creation and verification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        printf("Socket creation failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully created..\n");

    // initialize the socket addresses
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(SERVER_IP);
    servaddr.sin_port = htons(PORT);

    // Connection
    connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

    printf("Waiting to receive..\n\n");
    int count = 0;

    while (count < MAX_FRAMES) // max 30 frames can be sent
    {
        // receive message from the socket
        n = read(sockfd, buff2, MAX);
        if (n > 0)
        {
            // check the first letter for seq
            if (next_seq == (buff2[0] - '0'))
            {
                // skip all the characters before $
                for (i = 0; i < strlen(buff2); i++)
                {
                    q = buff2[i];
                    if (strcmp(&q, &del) == 0)
                        break;
                }

                // store all characters between $ (including) in buff3
                for (j = i; j < strlen(buff2); j++)
                {
                    buff3[p] = buff2[j];
                    p++;
                    w = buff2[j];
                    if (strcmp(&w, &del) == 0)
                        break;
                }
                frame->seq = (buff2[0] - '0');
                frame->type = DATA;
                frame->msg = buff3;
                frame->len = strlen(buff3);
                printf("Received frame %d\t", frame->seq);

                // toggle the seq to get ack
                ack = (frame->seq + 1) % 2;

                sprintf(buff1, "%d", ack);
                m = strlen(buff1) + 1;

                // wait for a random time before writing the ack packet
                srand(time(NULL));
                int sl = rand() % 4;
                sleep(sl);

                // write to the socket
                write(sockfd, buff1, m);
                printf("Ack %d sent\n", ack);

                // toggle between 0 and 1
                next_seq = (next_seq + 1) % 2;
                count++;
            }
        }

        if (n == 0)
        {
            printf("\nTransfer successful\nClient exit...\n");
            break;
        }
    }
    return 0;
}