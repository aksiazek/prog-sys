/**
 *  First readers-writers problem.
 */
#include <stdio.h>

#include <pthread.h>
#include <unistd.h>

#include "common.h"

#define READER_COUNT 3
#define ACTION_COUNT 4

pthread_mutex_t writerMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t readerMutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct {
	pthread_mutex_t readerMutex, writerMutex;
	int object;
} shared_data;

typedef struct {
	shared_data* shared;
	int threadID;
} thread_data;

int reader(void* data) {
	static int readersCount = 0;
	//pthread_mutex_t readerMutex = ((thread_data*) data)->shared->readerMutex;
	//pthread_mutex_t writerMutex = ((thread_data*) data)->shared->writerMutex;
	int* object = &(((thread_data*) data)->shared->object);
	int threadID = ((thread_data*) data)->threadID;
	printf("(R %d) Reader thread is born.\n", threadID);
	fflush(stdout);

	int i;
	for(i = 0; i < ACTION_COUNT; i++) {
		lock(&readerMutex);
		printf("(R %d) Reader locked readerMutex.\n", threadID);
		fflush(stdout);
		readersCount++;
		if (readersCount == 1) {
			lock(&writerMutex);
			printf("(R %d) Reader locked writerMutex.\n", threadID);
			fflush(stdout);
		}
		printf("(R %d) Reader unlocked readerMutex.\n", threadID);
		fflush(stdout);
		unlock(&readerMutex);
		
		printf("(R %d) Reader started reading.\n", threadID);
		fflush(stdout);
		usleep(randomInRange(200));
		printf("(R %d) Read %d.\n", threadID, *object);
		fflush(stdout);
		
		lock(&readerMutex);
		printf("(R %d) Reader locked readerMutex.\n", threadID);
		fflush(stdout);
		readersCount--;
		if (readersCount == 0) {
			printf("(R %d) Reader unlocked writerMutex.\n", threadID);
			fflush(stdout);
			unlock(&writerMutex);
		}
		printf("(R %d) Reader unlocked readerMutex.\n", threadID);
		fflush(stdout);
		unlock(&readerMutex);
		
		usleep(randomInRange(1000));
	}
	
	printf("(R %d) Reader thread exits.\n", threadID);
	fflush(stdout);
	return 0;
}

int writer(void* data) {
	//pthread_mutex_t writerMutex = ((shared_data*) data)->writerMutex;
	int* object = &(((shared_data*) data)->object);
	printf("(W) Writer thread is born.\n");
	fflush(stdout);
	
	int i;
	for(i = 0; i < ACTION_COUNT; i++) {
		lock(&writerMutex);
		
		printf("(W) Writer started writing\n");
		fflush(stdout);
		*object = randomInRange(100);
		usleep(randomInRange(800));
		printf("(W) Written %d.\n", *object);
		fflush(stdout);
			
		unlock(&writerMutex);
		
		usleep(randomInRange(1000));
	}
	
	printf("(W) Writer thread exits.\n");
	fflush(stdout);
	return 0;
}

int main(int argc, char* argv[]) {
	pthread_t writerThread;
	pthread_t readerThreads[READER_COUNT];
	
	shared_data shared;
	shared.object = 0;
	//shared.readerMutex = PTHREAD_MUTEX_INITIALIZER;
	//shared.writerMutex = PTHREAD_MUTEX_INITIALIZER;
	handleError(pthread_mutex_init(&(shared.readerMutex), NULL), "Error creating mutex");
	handleError(pthread_mutex_init(&(shared.writerMutex), NULL), "Error creating mutex");
	
	handleError(pthread_create(
		&writerThread, 		// thread identifier
		NULL,               // thread attributes
		(void*) writer,     // thread function
		(void*) &shared),   // thread function argument
	"Couldn't create the writer thread");

	int i;
	thread_data readers[READER_COUNT]; 
	for(i = 0; i < READER_COUNT; i++) {
		readers[i].shared = &shared;
		readers[i].threadID = i+1;
		handleError(pthread_create(
			&readerThreads[i], 	   	// thread identifier
			NULL,             		// thread attributes
			(void*) reader,    		// thread function
			(void*) &readers[i]),   // thread function argument
		"Couldn't create the reader threads");
	}

	for (i = 0; i < READER_COUNT; i++)
		pthread_join(readerThreads[i], NULL);
	pthread_join(writerThread, NULL);
	
	return 0;
}

