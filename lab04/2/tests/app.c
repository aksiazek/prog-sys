#include <sys/syscall.h>
#include <linux/unistd.h>
#include <stdio.h>
#include <errno.h>

#define __NR_mysyscall 317

long mysyscall(char* name_buf, long id) {
  syscall(__NR_mysyscall, name_buf, id);
}

int main(int argc, char* argv[])
{
	char process_name[16];
	long id = 0;
    while(mysyscall(process_name, id) == 0)
    {
		id++;
		printf("%s\n", process_name);
	}
    
    return 0;
}


