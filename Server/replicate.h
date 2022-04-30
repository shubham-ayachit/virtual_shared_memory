#ifndef CLIENT_H_INCLUDED
#define CLIENT_H_INCLUDED
#define _POSIX_SOURCE
#define _POSIX_C_SOURCE 199309L

#include<sys/types.h>
#include<netinet/in.h>
#include<errno.h>
#include<time.h>
#include<pthread.h>
#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<string.h>
#include <stdarg.h>
#include<signal.h>
#include<syscall.h>
#include<pthread.h>

#define MEMSIZE 1024


void tcplog(const char *fmt, ... ) {
	FILE* logfiletcp;
	logfiletcp = fopen("tcp.log", "w");
	va_list args;

    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);

    va_start(args, fmt);
    vfprintf(logfiletcp, fmt, args);
    va_end(args);
	fclose(logfiletcp);
}

void udplog(const char *fmt, ... ) {
	FILE* logfileudp; 
	logfileudp = fopen("udp.log", "w");
	va_list args;

    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);

    va_start(args, fmt);
    vfprintf(logfileudp, fmt, args);
    va_end(args);
	fclose(logfileudp);
}

//struct to send data to thread process while creating thread.
struct arg_struct {
	int sockfd;
	struct sockaddr *pcliaddr;
	int len;
	char *loc;
	char *buf;
	char *num;
	char *vec_clk;
	char *action;
};

struct serv_arg {
	char *sendline;
	char *recvline;
	char *num;
	char *vec_clock;
	char *server;
};

static char shared_array[MEMSIZE] = {0};
static int k = 0, l = 0;

static int vector_clock = 1;
pthread_mutex_t vec_mutex = PTHREAD_MUTEX_INITIALIZER;
//mutex array for lock/unlock that is of same size of shared memory.
static pthread_mutex_t mutex[MEMSIZE];
int lk[1024];

void mutex_init() {
	pthread_mutexattr_t mt;
	pthread_mutexattr_setpshared(&mt, PTHREAD_MUTEX_RECURSIVE_NP);
	for(int i = 0; i<1024; i++) {
		pthread_mutex_init(&mutex[i], &mt);
	}
}

void mutex_destroy() {
	
	for(int i = 0; i<1024; i++) {
		pthread_mutex_destroy(&mutex[i]);
	}
}

int inet_pton(int family, const char *strptr, void *addrptr);
ssize_t write(int fs, const void *buf, size_t N);
ssize_t read(int fs, void *buf, size_t N);

char servers[3][20];

void get_server_data() {
	FILE *f = fopen("server_details.txt", "r");
	int a = 0;
	// while(feof(f)) {
	// 	fscanf(f, "%*s %s", servers[a]);
	// 	a++;
	// }

	while(fscanf(f, "%*s %s", servers[a]) == 1)
    {
        a++;
    }

	fclose(f);
}

//void sendToServer(char sendline[], char recvline[], int num) {
void* sendToServer(void *arg) {

	struct serv_arg arguments = *(struct serv_arg *)arg;

	char r[atoi(arguments.num)]; 
    int sockfd, re;
	struct sockaddr_in servaddr;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(6214);
	inet_pton(AF_INET, arguments.server, &servaddr.sin_addr);

	if((re = connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr))) != 0) {
		if(re == ETIMEDOUT)
		return arguments.recvline;
	}

	udplog("In udp: sending to\t%s\tdata-%s\n", arguments.server, arguments.sendline);
	write(sockfd, arguments.sendline, strlen(arguments.sendline));

	int n = (read(sockfd, r, atoi(arguments.num)));
	sprintf(arguments.recvline, "%s", r);
	udplog("In udp: update from\t%s\tdata-%s\n", arguments.server, arguments.recvline);
    close(sockfd);
	//return;
	return arguments.recvline;

}

//function to send write request.
int WriteNMem(int loc, char *buf, int num) {
	tcplog("%s\n", "In WriteNMem");
	char *vec_clock;
	int rVal = 0;
	char sendline[MEMSIZE] = {0}; 
	char recvline[num];
	memset(recvline, 0, num); 
	char numm[num];
    sprintf(sendline,"%d", loc);
    sprintf(numm, "%d", num);
	
	strcat(sendline,",");
	strcat(sendline,buf);
	strcat(sendline,",");
    strcat(sendline, numm);
    strcat(sendline,",");

	if (asprintf(&vec_clock, "%d", vector_clock) == -1) {
        perror("asprintf");
    } else {
        strcat(sendline, vec_clock);
    }

	strcat(sendline,",");
	strcat(sendline,"1");
 
	get_server_data();
	if(servers[0][0] == '\0') {
		return 1;
	}
	pthread_t tid[3];
	char *number = calloc(4,sizeof(char));
	sprintf(number, "%d", num);
	sprintf(number, "%d", num);
	for(int l=0; l<3;l++) {
		tcplog("%d %s\n", l, servers[l]);
		struct serv_arg arg = {sendline, recvline, number, vec_clock, servers[l]};
		pthread_create(&tid[l], NULL, (void *)&sendToServer, (void *) &arg);
	}
	
	char * arg = NULL;
	for(int a = 0; a<3; a++) {
		pthread_join(tid[a],(void**)&arg);
		// struct serv_arg arguments = *(struct serv_arg *)arg;
		if(strcmp(arg, buf) == 0) {
			rVal = 1;	
		} else {
			rVal = 0;
		}
	}
	strcpy(buf, arg);
	tcplog("Leaving WriteNMem - buf: %s, rVal - %d\n", buf, rVal);
    return rVal;
}

//fuction to read request
char* ReadNMem(int loc, char *b, int num) {
	tcplog("%s\n", "In ReadNMem");
	char *vec_clock;
	int rVal;
	char sendline[MEMSIZE], recvline[num];
	memset(recvline, 0, num); 
	char numm[num];
	sprintf(numm, "%d", num);
	sprintf(sendline, "%d", loc);
	strcat(sendline,",");
	strcat(sendline, b);
	strcat(sendline,",");
	strcat(sendline, numm);
	strcat(sendline,",");
	if (asprintf(&vec_clock, "%d", vector_clock) == -1) {
        perror("asprintf");
    } else {
        strcat(sendline, vec_clock);
    }

	strcat(sendline,",");
	strcat(sendline,"6");
	strcat(sendline, "\0");
	
	get_server_data();
	if(servers[0][0] == '\0') {
		return "0";
	}
	
	pthread_t tid[3];
	char *number = calloc(4,sizeof(char));
	sprintf(number, "%d", num);
	for(int l=0; l<3;l++) {
		struct serv_arg arg = {sendline, recvline, number, vec_clock, servers[l]};
		pthread_create(&tid[l], NULL, (void *)&sendToServer, (void *) &arg);
	}
	
	char * arg = NULL;
	char *data[2];
	for(int a = 0; a<3; a++) {
		pthread_join(tid[a],(void**)&arg);
		// struct serv_arg arguments = *(struct serv_arg *)arg;
		if(strcmp(arg, b) == 0) {
			rVal = 0;	
		} else {
			char *token = strtok(arg, ",");
			int i = 0;
        	while (token) {
            	data[i++] = token;
            	token = strtok (NULL, ",");
        	}
		}
	}

	strcpy(b, arg);
	if(&data[2] == NULL) {
		char *ret = calloc(1, sizeof(char));
		sprintf(ret, "%d", rVal);
		tcplog("%s\t%s\n", "Leaving ReadNMem - ret:", ret);
		return ret;
	} else {
		tcplog("%s\t%s\n", "Leaving ReadNMem - buf:", arg);
		return arg;
	}
}



int lock_bytes(int loc, int num) {
	tcplog("Thread %ld - %s\n", pthread_self(), "In lock_bytes");
	int result = 1;

	struct timespec timeoutTime;
    clock_gettime(CLOCK_REALTIME, &timeoutTime);
    timeoutTime.tv_sec += 1;
	for(int j = loc; j<loc+num; j++) {
        //checking for lock
		result = pthread_mutex_timedlock(&mutex[j], &timeoutTime);
		while(result != 0 ) {
			if(lk[j] == 0) {
				pthread_mutex_unlock(&mutex[j]);
				result = pthread_mutex_timedlock(&mutex[j], &timeoutTime);
			} else {
				perror("Error locking bytes...\n");
				break;
			}
		}
	}
	tcplog("Thread %ld - %s\n", pthread_self(), "Leaving lock_bytes");
	return result;
}

void unlock_bytes(int loc, int num) {
	tcplog("%s\n", "In unlock_bytes");
	for(int j = loc; j<loc+num; j++) {
        //checking for unlock
		if(pthread_mutex_unlock(&mutex[j]) == 0) {
			lk[j];
		   	return;
		} else {
			perror("Error Unlocking bytes\n");
			return;
		}
	}
	tcplog("%s\n", "Leaving unloack");
}

//write from client
void write_update(int loc, int num, char *p, char *ptr, char rmesg[], int clk) {
	tcplog("Thread %ld - %s\n", pthread_self(), "In write_update");
	tcplog("%s\t%d\n", "Sender's vector_clock:",clk);
	int result = 1;
	struct timespec timeoutTime;
    clock_gettime(CLOCK_REALTIME, &timeoutTime);
    timeoutTime.tv_sec += 1;
	result = pthread_mutex_timedlock(&vec_mutex, &timeoutTime);
	if(result == 0 ) {
		if (clk == 0) {
			vector_clock++;
		} else if (vector_clock >= clk){
			vector_clock++;
		} else {
			vector_clock = clk;
			vector_clock++;
		}
		tcplog("Thread %ld - %s\t%d\n", pthread_self(), "Vector_clock",vector_clock);
	} else {
		perror("Error locking vector clock\n");
		return;
	}
	int i = 0;
	for(int j = loc; j<loc+num; j++) {
		*(p+j) = *ptr;
		rmesg[i++] = *ptr;
		ptr++;
	}
       rmesg[i]='\0';	
	pthread_mutex_unlock(&vec_mutex);
	unlock_bytes(loc, num);
	tcplog("write - loc %d, data: %s, num: %d\n", loc, rmesg, num);
	tcplog("Thread %ld - %s\n", pthread_self(), "Leaving write_update");
	return;
}


//read_from_client
void read_update(int loc, int num, char *p, char rmesg[], int clk) {
	tcplog("Thread %ld - %s\n", pthread_self(), "In read_update");
	tcplog("%s\t%d\n", "Sender's vector_clock:",clk);
	int result = 1;
	struct timespec timeoutTime;
    clock_gettime(CLOCK_REALTIME, &timeoutTime);
    timeoutTime.tv_sec += 1;
	result = pthread_mutex_timedlock(&vec_mutex, &timeoutTime);
	if(result == 0 ) {
		if(clk >= vector_clock) {
			vector_clock = clk;
		} else {
			vector_clock++;
		}
		tcplog("Thread %ld, Vector_clock\t%d\n", pthread_self(),vector_clock);
	} else {
		perror("Error locking vector clock\n");
		return;
	}
	int i = 0;
	for(int j = loc; j<loc+num; j++) {
		rmesg[i++] = *(p+j);
	}
	rmesg[i] = '\0';
	pthread_mutex_unlock(&vec_mutex);
	unlock_bytes(loc, num);
	tcplog("read - loc: %d, data: %s, num: %d", loc, rmesg, num);
	tcplog("Thread %ld - leaving read_update\n", pthread_self());
	return;
}

void child_function(char *data[], char rmesg[], char src[]) {
	tcplog("Thread %ld - %s\n", pthread_self(), "In child_function");
	//pointer to shared array.
	char *p;
	p = shared_array;
	int i = 0;

	//initializing request data
	int loc;
    loc = atoi(data[0]);
    int num, vec_clk;
    num = atoi(data[2]);
    char val[atoi(data[2])], *ptr;
    strcpy(val,  data[1]);
    ptr = val;
	vec_clk = atoi(data[3]);

	//switch call for various modification function.
    switch(atoi(data[4])) {

       	case 1 : //write given bytes from loc
		   		if(strcmp(src, "client") == 0) {
					if(lock_bytes(loc, num) == 0) {
					   int rVal = WriteNMem(loc, ptr, num);
					   if(rVal == 1) {
						   	write_update(loc, num, p, ptr, rmesg, 0);
							unlock_bytes(loc, num);
							tcplog("after update %s\n", rmesg);
					   } else {
						   rmesg = "Not updated on all servers...";
						   unlock_bytes(loc, num);
					   }
					}
				} else if(lock_bytes(loc, num) == 0) {
						write_update(loc, num, p, ptr, rmesg, vec_clk);
						unlock_bytes(loc, num);
				} else {
						udplog("%s\n", "Unable to lock and update.");
				}
		   		
                break;

		case 2 :
			//lock given bytes from loc
			lock_bytes(loc, num);
			break;

		case 3 : 
			//unloack given bytes
			for(int j = k; j<l; j++) {
                //checking for unlock
			    if(pthread_mutex_unlock(&mutex[j]) != 0) {
			    	perror("Error Unlocking bytes\n");
			    }
            }
            break;

		case 4 : 
			//lock all memory
			for(int j = 0; j<MEMSIZE; j++) {
                //lock check
			    if(pthread_mutex_lock(&mutex[j])!= 0) {
					perror("Error locking bytes\n");
                }
            }
			strcpy(rmesg, "locked");
            break;

		case 5 : 
			//unloacking all
			for(int j = 0; j<MEMSIZE; j++) {
                //unlock check
			    if(pthread_mutex_unlock(&mutex[j]) != 0) {
					perror("Error locking bytes\n");
                }
            }
			strcpy(rmesg, "unlocked");
            break;

		default : 
			if(strcmp(src, "client") == 0) {
					if(lock_bytes(loc, num) == 0) {
					   char *ret = ReadNMem(loc, ptr, num);
					   if(atoi(ret) == 0) {
						   	read_update(loc, num, p, rmesg, vec_clk);
							unlock_bytes(loc, num);
					   } else {
						   	char *rdata[2];
						   	char *token = strtok(ret, ",");
							int i = 0;
        					while (token) {
            				rdata[i++] = token;
            				token = strtok (NULL, ",");
        	}
						   rmesg = rdata[0];
						   tcplog("child_update - updating latest data - %s,\tloc:%d, number- %d\n", rmesg, loc, num);
						   tcplog("child_update - sender's vector clock - %s\n", rdata[1]);
						   (int) strtol(vec_clk, (char **)rdata[1], 10);
						   write_update(loc, num, p, rdata[0], rmesg, vec_clk);
						   unlock_bytes(loc, num);
					   }
					}
				} else if(lock_bytes(loc, num) == 0) {
						read_update(loc, num, p, rmesg, vec_clk);
						unlock_bytes(loc, num);
				} else {
					udplog("%s\n", "Unable to lock and update.");
				}
            break;
    }
	strcat(rmesg,"\0");
	tcplog("Threrad %ld - Leaving child_function - rmesg:\t%s\n", pthread_self(), rmesg);
	return;
}

#endif // CLIENT_H_INCLUDED
