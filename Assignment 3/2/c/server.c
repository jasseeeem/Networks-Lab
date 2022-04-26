#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/time.h>

#define PORT 8080
#define MAX 100
#define SERVER_IP "127.0.0.1"
#define MAX_FRAMES 30
#define SA struct sockaddr

typedef enum
{
    DATA,
    ACK
} FRAMEKIND;

struct timeval timeout;

struct Frame
{
    FRAMEKIND type;
    unsigned int len;
    int seq;
    char *msg;
};

struct Frame *makeframes()
{
    int i;
    int seqno = 1;
    struct Frame *fr = (struct Frame *)malloc(MAX_FRAMES * sizeof(struct Frame)); // 30 frames generated
    for (i = 0; i < MAX_FRAMES; i++)
    {
        fr[i].type = DATA;
        fr[i].seq = (seqno + 1) % 2;
        fr[i].msg = "$ubynb657benadbjlszncx.mvc/76s4e6?^&^rtyety7d5r7*F<xdcfvgl;kjhgfdsf345f$";
        fr[i].len = strlen(fr->msg);
        seqno = fr[i].seq;
    }
    return fr;
}
int main()
{
    int sockfd, connfd, num;
    struct sockaddr_in servaddr, cli;
    struct Frame *frm;
    struct Frame f;
    int n, len, k;
    char buff[MAX], buff1[MAX];
    pid_t pid;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

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

    int count = 0;

    while (count < MAX_FRAMES)
    {
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

        frm = makeframes();
        printf("\nEnter the number of frames to be sent (Maximum %d frames)\n>>> ", MAX_FRAMES);
        scanf("%d", &num);
        int next_ack;
        struct timeval rtt_sum;
        rtt_sum = (struct timeval){0};
        printf("\nSending frames...\n\n");

        // send
        while (count < num)
        {
            struct timeval send_time;
            gettimeofday(&send_time, NULL);
            if (setsockopt(connfd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof(timeout)) < 0)
            {
                perror("setsockopt(SO_RCVTIMEO) failed");
            }
            f = frm[count];
            sprintf(buff, "%d%d%s", f.seq, f.len, f.msg);
            if (f.len > 0)
            {
                k = write(connfd, buff, f.len);
                if (k > 0)
                {
                    printf("Sent Frame %d\t", f.seq);
                    // toggle ack
                    next_ack = ((f.seq) + 1) % 2;
                    n = read(connfd, buff1, MAX);
                    if (n == 0)
                        printf("No ack received\n");
                    if (n > 0)
                    {
                        struct timeval curr_time, diff_time;
                        gettimeofday(&curr_time, NULL);

                        timersub(&curr_time, &send_time, &diff_time);

                        timeradd(&rtt_sum, &diff_time, &rtt_sum);
                        printf("Received Ack %d\n", (buff1[0] - '0'));
                        if (next_ack == (buff1[0] - '0'))
                            count++;
                    }
                }
            }
        }
        close(connfd);
        printf("\nTransfer successful\nMessage size = %d bytes\nAverage RTT = %li microseconds\nServer exit...\n", frm[0].len, rtt_sum.tv_usec / num);
        if (count == num)
            break;
    }
    close(sockfd);
    return 0;
}