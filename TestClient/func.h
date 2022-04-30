#ifndef CLIENT_H_INCLUDED
#define CLIENT_H_INCLUDED

#include<sys/types.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<netinet/in.h>
//size of shared mem
#define MEMSIZE 1024

char servers[2][20];

void get_server_data() {
	FILE *f = fopen("server_ip.txt", "r");
	int a = 0;
	
        while(fscanf(f, "%*s %s", servers[a]) == 1) {
                a++;
        }

	fclose(f);
}

//convertion from presentation to network format
int inet_pton(int family, const char *strptr, void *addrptr);
void send_data(char[], char[] );

//struct to pass data 
struct arg_struct {
	int loc;
	char *buf[4];
	int num;
};

//Pointer array to keep track of server ips
char *serverip[] = {0};

void get_server_update() {

        FILE* f; 
        f = fopen("server_ip.txt", "r+");
        while(feof(f) != 1)
        fscanf(f, "%*s %s", serverip);
        //printf("%s", &serverip[0]);

        fclose(f);
}

//function to send write request.
int WriteNMem(int loc, char *buf, int num) {
    	int i, j;
	char sendline[MEMSIZE], recvline[num], numm[num];
    	sprintf(sendline,"%d", loc) ;
        sprintf(numm, "%d", num);
    	sendline[1] = ',';
        i = 0;
        while(buf[i] != '\0') {
                sendline[2+i] = *(buf+i);
                i++;
        }
	//for(i = 0; i<sizeof(&buf); i++) {
    	//	sendline[2+i] = *(buf+i);
    	//}
    	j = 2+i;
        i=0;
    	sendline[j++] = ',';
    	while(numm[i] != '\0') {
                sendline[j++] = numm[i++];
        }
        sendline[j++] = ',';
        sendline[j++] = '0';
    	sendline[j++] = ',';
    	sendline[j++] = '1';
        sendline[j] = '\0';
	//printf("%s\n", sendline);
    	send_data(sendline, recvline);
    	strcpy(buf, recvline);
    	return sizeof(recvline);
}

//fuction to read request
int ReadNMem(int loc, char *b, int num) {
	char sendline[MEMSIZE], recvline[num], numm[4];
	sprintf(numm, "%d", num);
	sprintf(sendline, "%d", loc);
	sendline[1] = ',';
	sendline[2] = ' ';
	sendline[3] = ',';
	sendline[4] = numm[0];
	sendline[5] = ',';
        sendline[6] = '0';
        sendline[7] = ',';
	sendline[8] = '6';
	
	send_data(sendline, recvline);
	strcpy(b, recvline);
	return sizeof(recvline);
}

//function to lock specific size.
int Llock(int loc, int num) {

	char sendline[MEMSIZE], recvline[num], numm[4];
        sprintf(numm, "%d", num);
        sprintf(sendline, "%d", loc);
        sendline[1] = ',';
        sendline[2] = ' ';
        sendline[3] = ',';
        sendline[4] = numm[0];
        sendline[5] = ',';
        sendline[6] = '0';
        sendline[7] = ',';
        sendline[8] = '2';
	send_data(sendline, recvline);
}

//function to unlock specific size
int Lunlock(int id) {

        char sendline[MEMSIZE], recvline[MEMSIZE];
        sprintf(sendline, "%d", id);
        sendline[1] = ',';
        sendline[2] = ' ';
        sendline[3] = ',';
        sendline[4] = ' ';
        sendline[5] = ',';
        sendline[6] = '0';
        sendline[7] = ',';
        sendline[8] = '3';
        send_data(sendline, recvline);
}

//functionn to lock all memory.
int Glock() {

        char sendline[MEMSIZE], recvline[MEMSIZE];
	sendline[0] = ' ';
	sendline[1] = ',';
        sendline[2] = ' ';
        sendline[3] = ',';
        sendline[4] = ' ';
        sendline[5] = ',';
        sendline[6] = '0';
        sendline[7] = ',';
        sendline[8] = '4';
        send_data(sendline, recvline);
}

//function to unlock all memory.
int Gunlock() {

        char sendline[MEMSIZE], recvline[MEMSIZE];
        sendline[0] = ' ';
        sendline[1] = ',';
        sendline[2] = ' ';
        sendline[3] = ',';
        sendline[4] = ' ';
        sendline[5] = ',';
        sendline[6] = '0';
        sendline[7] = ',';
        sendline[8] = '5';
        send_data(sendline, recvline);
}


//function to send data over UDP
void send_data(char sendline[], char recvline[]) {

    get_server_data();
    srand(time(NULL));
    int n = rand() % 2;

    char r[1024] = {0};

    int sockfd;
    //get_server_update();
    char address[] = {0};
    sprintf(address, "%s", servers[n]);

    //sprintf("141.219.64.105", "%s", address);
    printf("%s", address);
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(7844);
    inet_pton(AF_INET, address, &servaddr.sin_addr);
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    int i = 0;
    int c;
    c = connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    while(c < 0 ) {
    	perror("Connection failed... trying again");
        if(i<5) {
           c = connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
        }
        i++;
    }

    //printf("%s", sendline);
    write(sockfd, sendline, strlen(sendline));
    n = read(sockfd, r, sendline[2]);
    snprintf(recvline, sizeof(r), "%s", r);


    return;

}

#endif // CLIENT_H_INCLUDED

