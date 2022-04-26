#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/time.h>

#define MAX 500
#define PORT 4035
#define SA struct sockaddr_in

void send_file(FILE *fp, int sockfd)
{
	struct timeval start_time, end_time, sub_time;
	gettimeofday(&start_time, NULL);
	int n;
	char filePacket[MAX];
	memset(filePacket, 0, MAX);
	while ((n = fread(filePacket, sizeof(char), MAX, fp)) > 0)
	{
		if (n != MAX && ferror(fp))
		{
			perror("Read File Error");
			exit(1);
		}
		if (write(sockfd, filePacket, n) == -1)
		{
			perror("Cannot send the file");
			exit(1);
		}
		memset(filePacket, 0, MAX);
	}
	gettimeofday(&end_time, NULL);
	timersub(&end_time, &start_time, &sub_time);
	printf("File transferred successfully\nTime elapsed: %ld microseconds\n\n", sub_time.tv_usec);
	return;
}

void write_file(int sockfd, char *file_name)
{
	struct timeval start_time, end_time, sub_time;
	gettimeofday(&start_time, NULL);

	// file where the received data should be stored in
	ssize_t total = 0;
	char buff[MAX];

	// read the first 500 bytes
	int num_bytes = read(sockfd, &buff, MAX);
	if (num_bytes == -1)
	{
		perror("Read Error");
		exit(1);
	}
	long long int x = atoi(buff);
	if (x == 0)
	{
		printf("File does not exist\n");
		return;
	}
	FILE *fp = fopen(file_name, "w");

	// reading 500 bytes at a time
	while (total <= x)
	{
		// read next 500 bytes
		num_bytes = read(sockfd, buff, MAX);
		if (num_bytes == -1)
		{
			perror("Read File Error");
			exit(1);
		}

		if (x - total < MAX)
			num_bytes = x - total;

		total += num_bytes;

		// write the 500 bytes of data into the file
		if (fwrite(&buff, 1, num_bytes, fp) != num_bytes)
		{
			perror("Write File Error");
			exit(1);
		}

		// number of bytes read is more than than the file size
		if (total >= x)
			break;
		memset(buff, 0, sizeof(buff));
	}
	fclose(fp);
	gettimeofday(&end_time, NULL);
	timersub(&end_time, &start_time, &sub_time);
	printf("File received successfully\nTime elapsed: %ld microseconds\n\n", sub_time.tv_usec);
	return;
}

int main()
{
	int sockfd, connfd;
	struct sockaddr_in servaddr, cli;
	char buff1[MAX], buff2[MAX];
	int n = 0;

	do
	{
		printf(">>> ");
		fgets(buff1, 32, stdin);
		if (strncmp(buff1, "START", 5) != 0)
			printf("505 - Command not supported\n");
		else
			n = 1;
	} while (n != 1);

	// socket create and verification
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1)
	{
		printf("Socket creation failed\n");
		exit(1);
	}

	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_addr.s_addr = INADDR_ANY;
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(PORT);

	// Connect to server
	if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0)
	{
		printf("Could not connect to the server\n");
		exit(0);
	}
	else
		printf("200 - OK Connection is set up\n\n");

	memset(buff1, 0, sizeof(buff1));
	memset(buff2, 0, sizeof(buff2));
	int nb;

	while (1)
	{
		memset(buff1, 0, MAX);
		memset(buff2, 0, MAX);
		printf(">>> ");
		fgets(buff1, MAX, stdin);
		write(sockfd, buff1, strcspn(buff1, "\n"));
		read(sockfd, &buff2, MAX);
		printf("%s\n\n", buff2);
		if (strncmp(buff2, "300", 3) == 0)
			break;
	}
	while (1)
	{
		memset(buff1, 0, MAX);
		memset(buff2, 0, MAX);
		printf(">>> ");
		fgets(buff1, MAX, stdin);
		write(sockfd, buff1, strcspn(buff1, "\n"));
		read(sockfd, &buff2, MAX);
		printf("%s\n\n", buff2);
		if (strncmp(buff2, "305", 3) == 0)
			break;
	}
	while (1)
	{
		memset(buff1, 0, MAX);
		memset(buff2, 0, MAX);
		printf(">>> ");
		fgets(buff1, MAX, stdin);
		if (strncmp(buff1, "StoreFile", 9) == 0)
		{
			for (int i = 0; i < MAX; i++)
			{
				if (buff1[i] == '\n')
				{
					buff1[i] = ' ';
					break;
				}
			}
			write(sockfd, buff1, MAX);
			char *file_name = strtok(buff1, " ");
			file_name = strtok(NULL, " ");
			FILE *fp = fopen(file_name, "r");
			if (!fp)
			{
				printf("File does not exist\n\n");
				continue;
			}
			else
			{
				struct stat st;
				memset(buff2, 0, MAX);
				// get the file and store it in the buff2 st
				stat(file_name, &st);
				// store the file size in buff
				sprintf(buff2, "%ld", st.st_size);
				// send the file size
				write(sockfd, buff2, MAX);
				// send the actual file 500 bytes at a time
				send_file(fp, sockfd);
			}
			continue;
		}
		if (strncmp(buff1, "GetFile", 7) == 0)
		{
			write(sockfd, buff1, strcspn(buff1, "\n"));
			char *file_name = strtok(buff1, " ");
			file_name = strtok(NULL, " ");
			write_file(sockfd, file_name);
			continue;
		}
		write(sockfd, buff1, strcspn(buff1, "\n"));
		if (strncmp(buff1, "QUIT", 4) == 0)
		{
			write(sockfd, buff1, strcspn(buff1, "\n"));

			printf("Quitting the program\n");
			exit(0);
		}
		read(sockfd, &buff2, MAX);
		printf("%s\n\n", buff2);
	}
	close(sockfd);
}