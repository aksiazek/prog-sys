#include <sys/syscall.h>
#include <linux/unistd.h>
#include <stdio.h>
#include <errno.h>

#define __NR_mysyscall 317

unsigned int mysyscall(void) {
  syscall(__NR_mysyscall);
}

int main()
{
    printf("Result: %u\n", mysyscall());
    return 0;
}
