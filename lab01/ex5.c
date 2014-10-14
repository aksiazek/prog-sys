#include <stdio.h>

#include <pthread.h>
#include <unistd.h>

#include "common.h"

#define READERS_COUNT 8
#define WRITERS_COUNT 8
#define TURNS 4
#define OBJECT_COUNT 3

pthread_mutex_t objectMutexes[OBJECT_COUNT];
pthread_mutex_t objectDeepMutexes[OBJECT_COUNT];
int buffer[OBJECT_COUNT];
char full[OBJECT_COUNT];
int readerCount[OBJECT_COUNT];
int limits[OBJECT_COUNT];

int reader(void* data) {
	int threadId = *((int*) data);

	int i, j;
	for(i = 0; i < TURNS; i++) {
		for(j = 0; j < OBJECT_COUNT; j++) {
			int result = pthread_mutex_trylock(&objectMutexes[j]);
			if (result != 0)
				continue;
			
			if((full[j] == 0) || (readerCount[j] >= limits[j])) {
				unlock(&objectMutexes[j]);
				continue;
			}
			
			readerCount[j]++;
			if(readerCount[j] == 1)
				lock(&objectDeepMutexes[j]);
			unlock(&objectMutexes[j]);
			
			printf("(R %d) Reader started reading\n", threadId);
			fflush(stdout);
			usleep(randomInRange(200));
			printf("(R %d) Read %d in %d.\n", threadId, buffer[j], j);
			
			pthread_mutex_lock(&objectMutexes[j]);
			full[j] = 0;
			readerCount[j]--;
			if(readerCount[j] == 0)
				unlock(&objectDeepMutexes[j]);
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
			
			unlock(&objectMutexes[j]);
			lock(&objectDeepMutexes[j]);
				
			printf("(W %d) Writer started writing\n", threadId);
			fflush(stdout);
			usleep(randomInRange(800));
			buffer[j] = randomInRange(777);
			printf("(W%d) Written %d in %d.\n", threadId, buffer[j], j);
			
			unlock(&objectDeepMutexes[j]);
			
			lock(&objectMutexes[j]);
			full[j] = 1;
			unlock(&objectMutexes[j]);
		}

		usleep(randomInRange(1000));
	}
	
	return 0;
}

int main(int argc, char* argv[]) {
	pthread_t writerThreads[WRITERS_COUNT];
	pthread_t readerThreads[READERS_COUNT];
	
	int i;
	for (i = 0; i < OBJECT_COUNT; i++) {
		limits[i] = rand() % 5 + 1;	
		readerCount[i] = 0;
		full[i] = 0;
		pthread_mutex_init(&objectMutexes[i], NULL);
		pthread_mutex_init(&objectDeepMutexes[i], NULL);
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

