#include <stdio.h>

#include <pthread.h>
#include <unistd.h>

#include "common.h"

#define READERS_COUNT 7
#define WRITERS_COUNT 7
#define TURNS 4
#define OBJECT_COUNT 3

pthread_mutex_t objectMutexes[OBJECT_COUNT];
int buffer[OBJECT_COUNT];
char full[OBJECT_COUNT];
int readersCount;

int reader(void* data) {
	int threadId = *((int*) data);

	int i, j;
	for(i = 0; i < TURNS; i++) {
		
		for(j = 0; j < OBJECT_COUNT; j++) {
			int result = pthread_mutex_trylock(&objectMutexes[j]);
			if (result != 0)
				continue;
			
			if(full[j] == 0) {
				unlock(&objectMutexes[j]);
				continue;
			} 

			printf("(R %d) Reader started reading\n", threadId);
			fflush(stdout);
			usleep(randomInRange(200));
			printf("(R %d) Read %d.\n", threadId, buffer[j]);
			full[j] = 0;
			
			unlock(&objectMutexes[j]);
		}

		usleep(randomInRange(1000));
	}

	return 0;
}

int writer(void* data) {
	int threadId = *((int*) data);
	
	int i, j;
	for(i = 0; i < TURNS; i++) {
		for(j = 0; j < OBJECT_COUNT; j++) {
			int result = pthread_mutex_trylock(&objectMutexes[j]);
			if (result != 0)
				continue;
			
			if(full[j] == 1) {
				unlock(&objectMutexes[j]);
				continue;
			} 

			printf("(W %d) Writer started writing\n", threadId);
			fflush(stdout);
			usleep(randomInRange(800));
			buffer[j] = randomInRange(777);
			printf("(W%d) Written %d.\n", threadId, buffer[j]);
			full[j] = 1;
			
			unlock(&objectMutexes[j]);
		}

		usleep(randomInRange(1000));
	}
	
	return 0;
}

int main(int argc, char *argv[]) {
	pthread_t writerThreads[WRITERS_COUNT];
	pthread_t readerThreads[READERS_COUNT];
	
	int i;
	for (i = 0; i < OBJECT_COUNT; i++) {
		full[i] = 0;
		pthread_mutex_init(&objectMutexes[i], NULL);
	}
	
	int writerIDs[WRITERS_COUNT];
	for(i = 0; i < WRITERS_COUNT; i++) {
		writerIDs[i] = i+1;
		handleError(pthread_create(
			&writerThreads[i], 	   	// thread identifier
			NULL,             		// thread attributes
			(void*) writer,    		// thread function
			(void*) &writerIDs[i]),   // thread function argument
		"Couldn't create the writer threads");
	}

	int readerIDs[WRITERS_COUNT];
	for(i = 0; i < READERS_COUNT; i++) {
		readerIDs[i] = i+1;
		handleError(pthread_create(
			&readerThreads[i], 	   	// thread identifier
			NULL,             		// thread attributes
			(void*) reader,    		// thread function
			(void*) &readerIDs[i]),   // thread function argument
		"Couldn't create the reader threads");
	}

	for (i = 0; i < READERS_COUNT; i++)
		pthread_join(readerThreads[i], NULL);
	for (i = 0; i < WRITERS_COUNT; i++)
		pthread_join(writerThreads[i], NULL);

	return 0;
}

