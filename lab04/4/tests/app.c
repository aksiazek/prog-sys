#include <sys/syscall.h>
#include <linux/unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#define __NR_mysyscall 317

long mysyscall(char* name_buf, unsigned int pid) {
  syscall(__NR_mysyscall, name_buf, pid);
}

int main(int argc, char* argv[])
{
	if(argc != 2) { 
		printf("Usage: %s [pid]\n", argv[0]);
		exit(-1);
	}
	char process_name[16];
	unsigned int pid = (unsigned int) atoi(argv[1]);
	// assume ok input from user..
    if(mysyscall(process_name, pid) == 0)
		printf("%s\n", process_name);
	else
		printf("Process with pid %u does not exist\n", pid);
    
    return 0;
}


