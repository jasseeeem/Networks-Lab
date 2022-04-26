#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <dirent.h>

#define MAX 500
#define PORT 4035
#define SA struct sockaddr

void send_file(FILE *fp, int sockfd)
{
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
	return;
}

void write_file(int sockfd, char *file_name)
{
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
		return;
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
	return;
}

int find_username(char *username)
{
	char buff1[MAX], buff2[MAX], *line = NULL;
	size_t len = 0;
	ssize_t read_line;
	FILE *login_cred = fopen("../logincred.txt", "r");
	if (login_cred == NULL)
	{
		printf("Could not open login credentials file");
		exit(1);
	}
	while ((read_line = getline(&line, &len, login_cred)) != -1)
	{
		char *token = strtok(line, ",");
		if (strcmp(token, username) == 0)
			return 1;
	}
	fclose(login_cred);
	return 0;
}

int find_password(char *username, char *password)
{
	char buff1[MAX], buff2[MAX], *line = NULL;
	size_t len = 0;
	ssize_t read_line;
	FILE *login_cred = fopen("../logincred.txt", "r");
	if (login_cred == NULL)
	{
		printf("Could not open login credentials file");
		exit(1);
	}
	while ((read_line = getline(&line, &len, login_cred)) != -1)
	{
		char *token = strtok(line, ",");
		if (token && strcmp(token, username) == 0)
		{
			token = strtok(NULL, " ");
			if (strcmp(token, password) == 0)
				return 1;
			return 0;
		}
	}
	fclose(login_cred);
	return 0;
}

int main()
{
	int sockfd, connfd, len;
	struct sockaddr_in servaddr, cli;
	char buff1[MAX], buff2[MAX];
	int n;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1)
	{
		printf("socket creation failed\n");
		exit(1);
	}
	else
		printf("Socket successfully created\n");

	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_addr.s_addr = INADDR_ANY;
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(PORT);

	// Binding newly created socket to given IP and verification
	if ((bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr))) != 0)
	{
		printf("Socket binding failed\n");
		exit(1);
	}
	else
		printf("Socket successfully binded\n");

	// Now server is ready to listen and verification
	if ((listen(sockfd, 5)) != 0)
	{
		printf("Listening failed\n");
		exit(1);
	}
	else
		printf("Server listening on port %d\n", PORT);
	len = sizeof(cli);

	// Accept the data packet from client and verification
	connfd = accept(sockfd, (SA *)&cli, &len);
	if (connfd < 0)
	{
		printf("Server acccept failed\n");
		exit(1);
	}
	else
		printf("Server acccepted the client");

	char *line = NULL;
	size_t len2 = 0;
	ssize_t nbb;
	int flag = 0;

	FILE *login_cred = fopen("../logincred.txt", "r");
	if (login_cred == NULL)
	{
		printf("Could not open login credentials file");
		exit(1);
	}
	else
	{
		while (1)
		{
			memset(buff1, 0, MAX);
			read(connfd, &buff1, MAX);
			char *token = strtok(buff1, " ");
			if (strcmp(token, "USERN") == 0)
			{
				token = strtok(NULL, " ");
				if (token)
				{
					if (find_username(token))
					{
						char username[MAX];
						strcpy(username, token);
						strcpy(buff2, "300 - Correct Username; Need password");
						write(connfd, buff2, strlen(buff2));
						while (1)
						{
							memset(buff1, 0, MAX);
							read(connfd, &buff1, MAX);
							char *token2 = strtok(buff1, " ");
							if (strcmp(token2, "PASSW") == 0)
							{
								token2 = strtok(NULL, " ");
								if (token2)
								{
									if (find_password(username, token2))
									{
										strcpy(buff2, "305 - User Authenticated with password\n");
										strcat(buff2, "Welcome ");
										strcat(buff2, username);
										strcat(buff2, "!");
										write(connfd, buff2, strlen(buff2));
										break;
									}
									else
										strcpy(buff2, "310 - Incorrect password");
								}
								else
									strcpy(buff2, "505 - Command not supported");
							}
							else
								strcpy(buff2, "505 - Command not supported");
							write(connfd, buff2, strlen(buff2));
						}
						break;
					}
					else
						strcpy(buff2, "301 - Incorrect Username");
				}
				else
					strcpy(buff2, "505 - Command not supported");
			}
			else
				strcpy(buff2, "505 - Command not supported");
			write(connfd, buff2, strlen(buff2));
		}
		while (1)
		{
			memset(buff1, 0, MAX);
			memset(buff2, 0, MAX);
			read(connfd, &buff1, MAX);
			char *token = strtok(buff1, " ");
			if (token)
			{
				if (strcmp(token, "CreateFile") == 0)
				{
					token = strtok(NULL, " ");
					if (token)
					{
						if (fopen(token, "w"))
							strcpy(buff2, "File created successfully");

						else
							strcpy(buff2, "File could not be created");
					}
					else
						strcpy(buff2, "505 - Command not supported");
				}
				else if (strcmp(token, "ListDir") == 0)
				{
					DIR *d;
					struct dirent *dir;
					d = opendir(".");
					if (d)
					{
						while ((dir = readdir(d)) != NULL)
						{
							if (dir->d_type == DT_REG)
							{
								strcat(buff2, "\n");
								strcat(buff2, dir->d_name);
							}
						}
						closedir(d);
					}
				}
				else if (strcmp(token, "StoreFile") == 0)
				{
					token = strtok(NULL, " ");
					write_file(connfd, token);
					continue;
				}
				else if (strcmp(token, "GetFile") == 0)
				{
					token = strtok(NULL, " ");
					FILE *fp = fopen(token, "r");
					if (!fp)
					{
						strcpy(buff2, "0");
						write(connfd, buff2, MAX);
						continue;
					}
					else
					{
						struct stat st;
						memset(buff2, 0, MAX);
						// get the file and store it in the buff2 st
						stat(token, &st);
						// store the file size in buff
						sprintf(buff2, "%ld", st.st_size);
						// send the file size
						write(connfd, buff2, MAX);
						// send the actual file 500 bytes at a time
						send_file(fp, connfd);
					}
					continue;
				}
				else if (strcmp(token, "QUIT") == 0)
				{
					break;
				}
				else
				{
					strcpy(buff2, "505 - Command not supported");
				}
			}
			else
			{
				strcpy(buff2, "505 - Command not supported");
			}
			write(connfd, buff2, sizeof(buff2));
		}
	}
	close(sockfd);
}