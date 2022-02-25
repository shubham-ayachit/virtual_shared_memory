#ifndef CLIENT_H_INCLUDED
#define CLIENT_H_INCLUDED

#include<sys/types.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<netinet/in.h>

#define MEMSIZE 1024

int inet_pton(int family, const char *strptr, void *addrptr);
void send_data(char[], char[] );

int WriteNMem(int loc, char *buf, int num) {
        int i, j;
        char sendline[MEMSIZE], recvline[num], numm[1];
        sprintf(numm, "%d", num);

        sprintf(sendline,"%d", loc) ;
        sendline[1] = ',';
        for(i = 0; i<num; i++) {
                sendline[2+i] = *(buf+i);
        }
        j = 2+i;
        sendline[j++] = ',';
        sendline[j++] = numm[0];
        sendline[j++] = ',';
        sendline[j++] = '1';

        send_data(sendline, recvline);
        strcpy(buf, recvline);
        return sizeof(recvline);
}

int ReadNMem(int loc, char *b, int num) {
        char sendline[MEMSIZE], recvline[num], numm[1];
        sprintf(numm, "%d", num);
        sprintf(sendline, "%d", loc);
        sendline[1] = ',';
        sendline[2] = ' ';
        sendline[3] = ',';
        sendline[4] = numm[0];
        sendline[5] = ',';
        sendline[6] = '6';

        send_data(sendline, recvline);
        strcpy(b, recvline);
        return sizeof(recvline);
}

int Llock(int loc, int num) {

        char sendline[MEMSIZE], recvline[num], numm[1];
        sprintf(numm, "%d", num);
        sprintf(sendline, "%d", loc);
        sendline[1] = ',';
        sendline[2] = ' ';
        sendline[3] = ',';
        sendline[4] = numm[0];
        sendline[5] = ',';
        sendline[6] = '2';
        send_data(sendline, recvline);
}

int Lunlock(int id) {

        char sendline[MEMSIZE], recvline[50];
        sprintf(sendline, "%d", id);
        sendline[1] = ',';
        sendline[2] = ' ';
        sendline[3] = ',';
        sendline[4] = ' ';
        sendline[5] = ',';
        sendline[6] = '3';
        send_data(sendline, recvline);
}

int Glock() {

        char sendline[MEMSIZE], recvline[50];
        sendline[0] = ' ';
        sendline[1] = ',';
        sendline[2] = ' ';
        sendline[3] = ',';
        sendline[4] = ' ';
        sendline[5] = ',';
        sendline[6] = '4';
        send_data(sendline, recvline);
}

int Gunlock() {

        char sendline[MEMSIZE], recvline[50];
        sendline[0] = ' ';
        sendline[1] = ',';
        sendline[2] = ' ';
        sendline[3] = ',';
        sendline[4] = ' ';
        sendline[5] = ',';
        sendline[6] = '5';
        send_data(sendline, recvline);
}

void send_data(char sendline[], char recvline[]) {

    int sockfd, n;
    const char address[] = "141.219.64.105";
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(7844);
    inet_pton(AF_INET, address, &servaddr.sin_addr);
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if(connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 ) {
        perror("Connection failed");
    }

    write(sockfd, sendline, strlen(sendline));
    n = read(sockfd, recvline, sendline[2]);

    return;

}

#endif // CLIENT_H_INCLUDED