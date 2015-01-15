#ifndef STATS_OF_RANDOM_H_INCLUDED
#define STATS_OF_RANDOM_H_INCLUDED

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/file.h>
#include <linux/mman.h>
#include <linux/binfmts.h>
#include <linux/version.h>
#include <linux/utsname.h>
#include <linux/kallsyms.h>
#include <linux/dcache.h>
#include <linux/fs.h>
#include <linux/jiffies.h>
#include <linux/sysctl.h>
#include <linux/err.h>
#include <linux/namei.h>
#include <linux/fs_struct.h>
#include <linux/mount.h>
#include <asm/uaccess.h>
#include <asm/insn.h>

#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/proc_fs.h>
#include <linux/fcntl.h>

#include <linux/path.h>
#include <linux/namei.h>


MODULE_AUTHOR("Aleksander Ksiazek");
MODULE_DESCRIPTION("Random device extra statistics Module");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");

#define MODULE_NAME "stats_of_random"
#define PKPRE "[" MODULE_NAME "] "

#define MAX_FILE_LEN 256

#define OP_JMP_SIZE 5

#define IN_ERR(x) (x < 0)

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 28)
#define get_task_uid(task) task->uid
#define get_task_parent(task) task->parent
#else
#define get_task_uid(task) task->cred->uid
#define get_task_parent(task) task->real_parent
#endif

struct kernsym {
	void *addr; // orig addrc
	void *end_addr;
	unsigned long size;
	char *name;
	bool name_alloc; // whether or not we alloc'd memory for char *name
	u8 orig_start_bytes[OP_JMP_SIZE];
	void *new_addr;
	unsigned long new_size;
	bool found;
	bool hijacked;
	void *run;
};

struct symhook {
	char *name;
	struct kernsym *sym;
	unsigned long *func;
};

int simple_open(struct inode *inode, struct file *filp);
int simple_release(struct inode *inode, struct file *filp);
ssize_t simple_read(struct file *filp, char *user_buf, size_t count, loff_t *f_pos);
ssize_t simple_write(struct file *filp, const char *user_buf, size_t count, loff_t *f_pos);
ssize_t simple_read_proc(struct file *filp, char *user_buf, size_t count, loff_t *f_pos);
void simple_exit(void);
int simple_init(void);
ssize_t random_read_awesome(struct file *file, char __user *buf, size_t nbytes, loff_t *ppos);

int symbol_hijack(struct kernsym *, const char *, unsigned long *);
void symbol_restore(struct kernsym *);

void hijack_syscalls(void);
void undo_hijack_syscalls(void);

void symbol_info(struct kernsym *);
int find_symbol_address(struct kernsym *, const char *);

int kernfunc_init(void);
void tpe_insn_init(struct insn *, const void *);
void tpe_insn_get_length(struct insn *insn);
int tpe_insn_rip_relative(struct insn *insn);
void *malloc(unsigned long size);
void malloc_free(void *buf);

#endif
