#include <stdio.h>

#include <pthread.h>
#include <unistd.h>

#include "common.h"

#define READERS_COUNT 7
#define WRITERS_COUNT 7
#define TURNS 4

pthread_mutex_t readerMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t writerMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t criticAway = PTHREAD_COND_INITIALIZER;
char wroteSomething[WRITERS_COUNT];
int writerCount; // you are not a writer till you have written something!
int readerCount;

int reader(void* data) {
	int threadId = *((int*) data);

	int i;
	for(i = 0; i < TURNS; i++) {
		
		lock(&readerMutex);
		readerCount++;
		if(readerCount == 1)
			lock(&writerMutex);
		unlock(&readerMutex);

		printf("(R %d) Reader started reading\n", threadId);
		fflush(stdout);
		usleep(randomInRange(200));
		printf("(R %d) Read.\n", threadId);

		lock(&readerMutex);
		readerCount--;
		if(readerCount == 0)
			unlock(&writerMutex);
		unlock(&readerMutex);

		usleep(randomInRange(1000));
	}

	return 0;
}

int writer(void* data) {
	int threadId = *((int*) data);
	
	int i;
	for(i = 0; i < TURNS; i++) {
		
		lock(&writerMutex);
			
		printf("(W %d) Writer started writing\n", threadId);
		fflush(stdout);
		usleep(randomInRange(800));
		printf("(W%d) Written.\n", threadId);
		if(wroteSomething[threadId] == 0) {
			wroteSomething[threadId] = 1;
			writerCount++;
		}
		
		if(writerCount == WRITERS_COUNT)
			pthread_cond_broadcast(&criticAway);
		
		unlock(&writerMutex);

		usleep(randomInRange(1000));
	}
	
	return 0;
}

int critic(void* data) {
	int i;
	for(i = 0; i < TURNS; i++) {
		
		lock(&writerMutex);
		while(writerCount != WRITERS_COUNT) {
			pthread_cond_wait(&criticAway, &writerMutex);
		}
			
		printf("(C) Critic starts criticizing\n");
		fflush(stdout);
		usleep(randomInRange(500));
		printf("(C) Criticized.\n");
		
		unlock(&writerMutex);

		usleep(randomInRange(1000));
	}

	return 0;
}

int main(int argc, char *argv[]) {
	pthread_t writerThreads[WRITERS_COUNT];
	pthread_t readerThreads[READERS_COUNT];
	pthread_t criticThread;
	
	int i;	
	for (i = 0; i < WRITERS_COUNT; i++)
		wroteSomething[i] = 0;
	
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

	handleError(pthread_create(
			&criticThread, 	   	// thread identifier
			NULL,             		// thread attributes
			(void*) critic,    		// thread function
			NULL),   // thread function argument
		"Couldn't create the critic thread");

	for (i = 0; i < READERS_COUNT; i++)
		pthread_join(readerThreads[i], NULL);
	for (i = 0; i < WRITERS_COUNT; i++)
		pthread_join(writerThreads[i], NULL);
	pthread_join(criticThread, NULL);

	return 0;
}

