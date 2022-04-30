#include "func.h"
#include<time.h>
#include<stdlib.h>
#include <pthread.h>
#include<string.h>

void* readMem(void *arg);
void* writeMem(void *arg);

//const string array for data to write in shared memory.
const char *data[15];

void init() {
	data[0] = "abcd";
	data[1] = "1234567";
	data[2] = "testdata";
	data[3] = "multi";
	data[4] = "mtu";
	data[5] = "testingenv";
	data[6] = "computing";	
}

int main(int argc, char * argv[]) {
	//initialized const array for data to write
	init();
	struct arg_struct s1;
	srand(time(NULL));
	pthread_t tid[15];
	for(int i = 0; i<15; i++) {
		int r = rand() % 6;
		int id = rand() % 15;
		
		//struct s1 is initiated for write and thread for write is created.
		s1.loc = id;
		char temp[sizeof(data[r])] = {0};
		s1.buf[0] = temp;
		snprintf(s1.buf[0], sizeof(data[r])-1, "%s", data[r]);
		s1.num = sizeof(s1.buf[0]);
		pthread_create(&tid[id], NULL, &writeMem, (void *) &s1);

		//s1 is modified for read and thread for read is created.
		id = rand() % 15;
		s1.loc = id;
                s1.num = id;
		pthread_create(&tid[id], NULL, &readMem, (void *) &s1);

		//thread for all lock is created.
		//id = rand() % 15;
		//pthread_create(&tid[id], NULL, &Glock, NULL);

		//thread for all unlock is created.
		//id = rand() % 15;
		//pthread_create(&tid[id], NULL, Gunlock, NULL);
    	}
}

//thread function to calll actual write from hreader file.
void* writeMem(void *arg) {
	
	struct arg_struct s1 = *(struct arg_struct *) arg;
	s1.num = WriteNMem(s1.loc, s1.buf[0], s1.num);
	printf("Write - loc:  %d, data: %s\n", 1, s1.buf[0]);
	pthread_exit(NULL);
}

//thread function to call actual read from header file.
void* readMem(void *arg) {
	struct arg_struct s1 = *(struct arg_struct *) arg;
	s1.num = ReadNMem(s1.loc, s1.buf[1], s1.num);
	printf("Read - loc: %d, data: %s\n", s1.num, s1.buf[1]);
	pthread_exit(NULL);
}
