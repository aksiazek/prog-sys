#include "module.h"

module_init(stats_of_random_init);
module_exit(stats_of_random_exit);

struct proc_dir_entry *proc_entry = NULL;

/* Operations for /proc entries */
struct file_operations proc_fops = {
	.read = all_bytes_read_proc
};

struct kernsym sym_random_read;

struct symhook random_read_hook = {
		"random_read", 
		&sym_random_read, 
		(unsigned long *)random_read,
};

ssize_t random_read(struct file *file, char __user *buf, size_t nbytes, loff_t *ppos)
{
	ssize_t (*run)(struct file*, char*, size_t, loff_t*) = sym_random_read.run;
	ssize_t bytes_read = run(file, buf, nbytes, ppos);
	
	printk(KERN_INFO "bytes read: %lu\n", (unsigned long) bytes_read);
	
	return bytes_read;
}

int stats_of_random_init(void) {
    int ret;
//    unsigned long random_read;

	ret = kernfunc_init();
	if (IN_ERR(ret))
		return ret;

    /* Register an entry in /proc/sys/kernel/random/ */
    proc_entry = proc_create("bytes_read", 0, NULL, &proc_fops);
    
//    random_read = kallsyms_lookup_name("random_read");
//    printk(KERN_INFO "%lu\n", random_read);
    
    ret = symbol_hijack(random_read_hook.sym, random_read_hook.name, random_read_hook.func);
	if (IN_ERR(ret))
		printk(KERN_ALERT "%s ERROR\n", random_read_hook.name);
		
	printk(KERN_INFO "The stats_of_random module has been inserted.\n");
	return 0;
}

void stats_of_random_exit(void) {
	if (proc_entry) {
		proc_remove(proc_entry);
	}
	
	symbol_restore(random_read_hook.sym);

    printk(KERN_INFO "The stats_of_random module has been removed\n");
}

ssize_t all_bytes_read_proc(struct file *filp, char *user_buf, size_t count, loff_t *f_pos) {
	return 0;
}
