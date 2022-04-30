#include<sys/types.h>
#include<netinet/in.h>
#include<errno.h>



int main(int argc, char **argv) {
    char sendline[] = "test";
    char recvline[1024] = {0};
    int sockfd;
	struct sockaddr_in servaddr;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(6211);
	inet_pton(AF_INET, "141.219.64.104", &servaddr.sin_addr);

	connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

	write(sockfd, (const void *) &sendline, strlen(sendline));

	read(sockfd, (void *)&recvline, 1024);
	printf("%s\n", recvline);

    close(sockfd);
}
