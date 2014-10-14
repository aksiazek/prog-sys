#include <stdarg.h>
#include <stdlib.h>

int randomInRange(int max) {
	return rand() % max;
}

void handleError(int result, const char* format, ...) {
	va_list args;
	va_start(args, format);
	if (result != 0) {
		vfprintf(stderr, format, args);
		exit(-1);
	}
	va_end(args);
}

void lock(pthread_mutex_t* mutex) {
	handleError(pthread_mutex_lock(mutex), "Error occured during locking mutex.\n");
}

void unlock(pthread_mutex_t* mutex) {
	handleError(pthread_mutex_unlock(mutex), "Error occured during unlocking mutex.\n");
}
