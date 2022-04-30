#include "replicate.h"
#include <unistd.h>

//size of shared array.

void* thread_process(void *arg) {
	struct arg_struct arguments = *(struct arg_struct *)arg;
	char *data[] = {arguments.loc, arguments.buf, arguments.num, arguments.vec_clk, arguments.action};
	
	//array for returning data to client.
	char rmesg[atoi(data[2])+1];
	char address[20] = {0};
	inet_ntop(AF_INET, arguments.pcliaddr->sa_data, &address);
	tcplog("%s\t%s\n", "address", address);
	//calling child function - function which modifies the shared array.
	child_function(data, rmesg, "client");
	//unlock_bytes(data[0], data[2]);
	//checking for loacking/unlocking calls.
	if(rmesg[0] == '\0') {
		sprintf(rmesg, "%d", arguments.sockfd);
	}

	//sending data to client as per request.
	
	int n = sendto(arguments.sockfd, rmesg, sizeof(rmesg), 0,(struct sockaddr *) arguments.pcliaddr, arguments.len);
	tcplog("%s\t%s\n", "Sending Client", rmesg);
	inet_ntop(AF_INET, arguments.pcliaddr->sa_data, &address);
	tcplog("%s\t%s\n", "client address", address);
	//exiting the thread.
	pthread_exit(NULL);
}

//Listener thrread for clients
void* udp_client_listner() {
	udplog("%s\t\n","In udp:");
	//char *p;
    char mesg[MEMSIZE];
    //p = shared_array;
    int sockfd, len;
    struct sockaddr_in servaddr, cliaddr;
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(7844);
	//get_server_data();

	bind(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

    for(;;) {
		//array for spliting data got from network.
	   	char *data[5];
        len = sizeof(cliaddr);
		//recv socket func.
        int n = recvfrom(sockfd, mesg, MEMSIZE, 0,(struct sockaddr *) &cliaddr, &len);
        char *token = strtok(mesg, ",");
        int i = 0;
        while (token) {
            data[i++] = token;
            token = strtok (NULL, ",");
        }
		sprintf(data[4], "%1s", data[4]);
		
		//struct to pass data to thread function.
		struct arg_struct arguments = {sockfd, (struct sockaddr *)&cliaddr, len, data[0], data[1], data[2], data[3], data[4]};
		
		pthread_t tid;

		//thread created for process - threadProcess
		pthread_create(&tid, NULL, &thread_process, (void *) &arguments);
    }	

    for(int j=0; j<MEMSIZE; j++) {
		pthread_mutex_destroy(&mutex[j]);
	}

	pthread_exit(NULL);
}

void* tcp_listner() {
    tcplog("%s\t\n",  "In tcp:");
    int listenfd;
    socklen_t clilen;
    struct sockaddr_in cliaddr, servaddr;
    char mesg[1024];

    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(6214);

    bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

    listen(listenfd, 1);

    for(;;) {
        clilen = sizeof(cliaddr);
        int connfd;
		connfd = accept(listenfd, (struct sockaddr *) &cliaddr, &clilen);
		tcplog("%s\t%s\n", "In tcp:","connected");
		char address[] = {0};
		inet_ntop(AF_INET, &cliaddr.sin_addr, &address);
		tcplog("%s %s\n", "In tcp: Update request from",address);
        if ((recv(connfd, &mesg, 50, 0)) > 0) {
			char *data[5];
			tcplog("%s\t%s\n", "In tcp: Recieved",mesg);
            char *token = strtok(mesg, ",");
            int i = 0;
            while (token) {
                data[i++] = token;
                token = strtok (NULL, ",");
            }
	    	sprintf(data[4],"%1s", data[4]);
		
	    	//tcplog("%s\n", data[0]);
	    	//tcplog("%s\n", data[1]);
	    	//tcplog("%s\n", data[2]);
	  		//tcplog("%s\n", data[3]);
			char rmesg[atoi(data[2])+1];
			child_function(data, rmesg, "server");
			//unlock_bytes(data[0], data[2]);
			char retr[atoi(data[2])+5];
			sprintf(retr, "%s", rmesg);
			int rVal = 0;
			rVal = strcmp(data[1], rmesg);
			clilen=sizeof(rmesg);
			if(rVal == 0 || atoi(data[3]) >= vector_clock) {
				tcplog("data check correct for %s, data-%s\n", address, data[1]);
				send(connfd, data[1], sizeof(data[1]), 0);
			} else if (atoi(data[3]) < vector_clock) {
				int i = sizeof(rmesg), j=0;
				char vec_clock[3] = {0};
				sprintf(vec_clock, "%d", vector_clock);
				retr[i] = ',';
				while(vec_clock[j] != '\0') {
					retr[i++] = vec_clock[j++];
				}
				retr[i] = '\0';
				udplog("In udp: sending to\t%s\tdata-%s\n", address, retr);
				send(connfd, retr, sizeof(retr), 0);
			}else {
				tcplog("data not update on %s\tsending - %s\n", address, "no updated");
				send(connfd, "not updated", 20, 0);
			}
			tcplog("%s\n", rmesg);	
		}
		tcplog("tcp connection closed with %s\n", address);
		close(connfd);
    }
    close(listenfd);
	pthread_exit(NULL);

}

int main()
{
	mutex_init();
    tcplog("%s\n", "Welcome...");
	pthread_t pid[2];

	pthread_create(&pid[0], NULL, &tcp_listner, NULL);
	pthread_create(&pid[1], NULL, &udp_client_listner, NULL);

    pthread_join(pid[0], NULL);
    pthread_join(pid[1], NULL);

	mutex_destroy();
    return 0;
}
