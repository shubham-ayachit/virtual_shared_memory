#include <stdio.h>
#include <stdlib.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<string.h>
#include <pthread.h>
#include<errno.h>
#define MEMSIZE 1024


struct arg_struct {
        int sockfd;
        struct sockaddr *pcliaddr;
        int len;
        char *loc;
        char *buf;
        char *num;
        char *action;
};

static char shared_array[MEMSIZE] = {0};
static int k = 0, l = 0;

pthread_rwlock_t locks[MEMSIZE] = {PTHREAD_RWLOCK_INITIALIZER};
pthread_mutex_t mutex[MEMSIZE] = {PTHREAD_MUTEX_INITIALIZER};

void child_function(char *data[], char rmesg[]) {
        char *p, rtrn[atoi(data[2])];
        p = shared_array;
        int i = 0;

        int loc;
        loc = atoi(data[0]);
        int num;
        num = atoi(data[2]);
        char val[atoi(data[2])], *ptr;
        strcpy(val,  data[1]);
        ptr = val;

        switch(atoi(data[3])) {

                case 1 : //write given bytes from loc
                        for(int j = loc; j<loc+num; j++) {
                            int result = pthread_mutex_trylock(&mutex[j]);
                            //if(pthread_rwlock_wrlock(&locks[j]) == 0) {
                            if(result == 0 ) {
                                *(p+j) = *ptr;
                                rmesg[i++] = *ptr;
                                ptr++;
                                //pthread_rwlock_unlock(&locks[j]);
                                pthread_mutex_unlock(&mutex[j]);
                            } else if (result == EBUSY) {
                                printf("%d\n", result);
                            }
                        }
                        break;

                case 2 : //lock given bytes from loc
                        for(int j = loc; j<loc+num; j++) {
                            //if ((pthread_rwlock_rdlock(&locks[j])) != 0 && (pthread_rwlock_wrlock(&locks[j])) !=0) {
                            if(pthread_mutex_lock(&mutex[j]) !=0 ) {
                                k = loc;
                                l = loc+num;
                                perror("Error locking bytes\n");
                            }
                        }
                        break;

                case 3 : //unloack given bytes
                        for(int j = k; j<l; j++) {
                            //if(pthread_rwlock_unlock(&locks[j]) != 0) {
                            if(pthread_mutex_unlock(&mutex[j]) != 0) {
                                perror("Error Unlocking bytes\n");
                            }
                        }
                        break;

                case 4 : //lock all memory
                        for(int j = 0; j<MEMSIZE; j++) {
                            //if ((pthread_rwlock_rdlock(&locks[j])) != 0 && (pthread_rwlock_wrlock(&locks[j]) !=0)) {
                            if(pthread_mutex_lock(&mutex[j])!= 0) {
                                perror("Error locking bytes\n");
                            }
                        }
                        strcpy(rmesg, "locked");
                        break;

                case 5 : //unloacking all
                        for(int j = 0; j<MEMSIZE; j++) {
                            //if (pthread_rwlock_unlock(&locks[j]) != 0) {
                            if(pthread_mutex_unlock(&mutex[j]) != 0) {
                                perror("Error locking bytes\n");
                            }
                        }
                        strcpy(rmesg, "unlocked");
                        break;

                default : //reading from loc
                        for(int j = loc; j<loc+num; j++) {
                                int result = pthread_mutex_trylock(&mutex[j]);
                                //if(pthread_rwlock_rdlock(&locks[j]) == 0) {
                                if(result == 0 ) {
                                        printf("%c\n",*(p+j));
                                        rmesg[i++] = *(p+j);
                                        //pthread_rwlock_unlock(&locks[j]);
                                        pthread_mutex_unlock(&mutex[j]);
                                } else if (result == EBUSY) {
                                        printf("%d\n",result);
                                }
                        }
                        break;
                }
        return;
}

void* thread_process(void *arg) {
        struct arg_struct arguments = *(struct arg_struct *)arg;
        char *data[] = {arguments.loc, arguments.buf, arguments.num, arguments.action};
        char rmesg[atoi(data[2])+1];
        child_function(data, rmesg);
        if(rmesg[0] == '\0') {
                sprintf(rmesg, "%d", arguments.sockfd);
        }
        sendto(arguments.sockfd, rmesg, sizeof(rmesg), 0,(struct sockaddr *) arguments.pcliaddr, arguments.len);
        pthread_exit(NULL);
}

int main()
{
        //char *p;
        char mesg[50];
        //p = shared_array;
        int sockfd, len, n, action;
        struct sockaddr_in servaddr, cliaddr;
        sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        memset(&servaddr, 0, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
        servaddr.sin_port = htons(7844);

        bind(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

        for(;;) {
                char *data[4];
                len = sizeof(cliaddr);
                n = recvfrom(sockfd, mesg, 50, 0,(struct sockaddr *) &cliaddr, &len);
                char *token = strtok(mesg, ",");
                int i = 0;
                while (token) {
                        data[i++] = token;
                        token = strtok (NULL, ",");
                }


                struct arg_struct arguments = {sockfd, (struct sockaddr *)&cliaddr, len, data[0], data[1], data[2], data[3]};
                //char rmesg[atoi(data[2])+1];
                pthread_t tid;
                pthread_create(&tid, NULL, &thread_process, (void *) &arguments);
        }

        for(int j=0; j<MEMSIZE; j++) {
                pthread_rwlock_destroy(&locks[j]);
        }
        return 0;
}