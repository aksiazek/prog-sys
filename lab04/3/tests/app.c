#include <sys/syscall.h>
#include <linux/unistd.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#define __NR_mysyscall 317

long long mysyscall(char* file_path, unsigned int len) {
  syscall(__NR_mysyscall, file_path, len);
}

int main(int argc, char* argv[])
{
	if(argc != 2) { 
		printf("Usage: %s [path_to_a_file]\n", argv[0]);
		exit(-1);
	}
	printf("len %u\n", (unsigned) strlen(argv[1]));
	long long result = mysyscall(argv[1], (unsigned) strlen(argv[1]));
	if(result >= 0) 
		printf("%llu\n", result);
	else
		printf("not a valid path\n");
    return 0;
}


