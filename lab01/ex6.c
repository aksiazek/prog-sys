#include <stdio.h>

#include <pthread.h>
#include <unistd.h>

#include "common.h"

#define READERS_COUNT 8
#define WRITERS_COUNT 8
#define TURNS 4
#define BUFFER_SIZE 4

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t readerCond = PTHREAD_COND_INITIALIZER; 
pthread_cond_t writerCond = PTHREAD_COND_INITIALIZER; 
int buffer[BUFFER_SIZE];
int written = 0;
int toRead = 0;
int writeIndex = 0;
int readIndex = 0;

int reader(void* data) {
	int threadId = *((int*) data);

	int i, j;
	for(i = 0; i < TURNS; i++) {
		
		lock(&mutex);
		while((written < BUFFER_SIZE) || (readIndex == BUFFER_SIZE)) {
			pthread_cond_wait(&readerCond, &mutex);
		}
		j = readIndex;
		readIndex++;
		unlock(&mutex);

		printf("(R %d) Reader started reading\n", threadId);
		fflush(stdout);
		usleep(randomInRange(200));
		printf("(R %d) Read %d in index %d.\n", threadId, buffer[j], j);
		
		lock(&mutex);
		toRead--;
		if(toRead == 0) {
			written = 0;
			writeIndex = 0;
			pthread_cond_broadcast(&writerCond);
		}
		unlock(&mutex);

		usleep(randomInRange(1000));
	}

	return 0;
}

int writer(void* data) {
	int threadId = *((int*) data);
	
	int i, j;
	for(i = 0; i < TURNS; i++) {
		
		lock(&mutex);
		while((toRead > 0) || (writeIndex == BUFFER_SIZE)) {
			pthread_cond_wait(&writerCond, &mutex);
		}
		j = writeIndex;
		writeIndex++;
		unlock(&mutex);

		printf("(W %d) Writer started writing\n", threadId);
		fflush(stdout);
		usleep(randomInRange(800));
		buffer[j] = randomInRange(777);
		printf("(W%d) Written %d in index %d.\n", threadId, buffer[j], j);
		
		lock(&mutex);
		written++;
		if(written == BUFFER_SIZE) {
			toRead = BUFFER_SIZE;
			readIndex = 0;
			pthread_cond_broadcast(&readerCond);
		}
			
		unlock(&mutex);

		usleep(randomInRange(1000));
	}
	
	return 0;
}

int main(int argc, char *argv[]) {
	pthread_t writerThreads[WRITERS_COUNT];
	pthread_t readerThreads[READERS_COUNT];
	
	int i;
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

