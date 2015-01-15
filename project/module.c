#include "module.h"

#define simple_major 199

struct proc_dir_entry *proc_entry = 0;
const char* mountpoint = 0;

module_init(simple_init);
module_exit(simple_exit);

/* Operations for /dev/simple */
struct file_operations simple_fops = {
    .read = simple_read,
    .write = simple_write,
    .open = simple_open,
    .release = simple_release
};

/* Operations for /proc/simple */
struct file_operations proc_fops = {
	.read = simple_read_proc
};

struct kernsym sym_random_read_analysed;

/*const struct file_operations my_random_fops = {
       .read  = sym_random_read.run,
       .write = sym_random_write.run,
       .poll  = sym_random_poll.run,
       .unlocked_ioctl = sym_random_ioctl.run,
       .fasync = sym_random_fasync.run,
       .llseek = sym_noop_llseek.run,
};*/

struct symhook random_read_analysed = {
		"random_read", 
		&sym_random_read_analysed, 
		(unsigned long *)random_read_awesome,
};

ssize_t random_read_awesome(struct file *file, char __user *buf, size_t nbytes, loff_t *ppos)
{
	
	
	ssize_t (*run)(struct file*, char*, size_t, loff_t*) = sym_random_read_analysed.run;
	ssize_t bytes_read = run(file, buf, nbytes, ppos);
	
	printk(KERN_INFO "bytes read: %lu\n", (unsigned long) bytes_read);
	
	return bytes_read;
}

int simple_init(void) {
    int result, ret;
    unsigned long random_read;

	ret = kernfunc_init();
	if (IN_ERR(ret))
		return ret;

    /* Register an entry in /proc */
    proc_entry = proc_create("simple", 0, NULL, &proc_fops);

    /* Register a device with the given major number */
    result = register_chrdev(simple_major, "simple", &simple_fops);
    if (result < 0) {
        printk(KERN_WARNING "Cannot register the /dev/simple device with major number: %d\n", simple_major);
        proc_remove(proc_entry);
        return result;
    }
    
    random_read = kallsyms_lookup_name("random_read");
    printk(KERN_INFO "%lu\n", random_read);
    
    ret = symbol_hijack(random_read_analysed.sym, random_read_analysed.name, 
		random_read_analysed.func);
	if (IN_ERR(ret))
		printk(KERN_ALERT "%s ERROR\n", random_read_analysed.name);
		
	random_read = kallsyms_lookup_name("random_read");
    printk(KERN_INFO "%lu\n", random_read);
    
	printk(KERN_INFO "The SIMPLE module has been inserted.\n");
	return 0;
}

void simple_exit(void) {
    /* Unregister the device and /proc entry */
    unregister_chrdev(simple_major, "simple");
	if (proc_entry) {
		proc_remove(proc_entry);
	}
	
	symbol_restore(random_read_analysed.sym);

    printk(KERN_INFO "The SIMPLE module has been removed\n");
}

int simple_open(struct inode *inode, struct file *filp) {
	try_module_get(THIS_MODULE);
    return 0;
}

int simple_release(struct inode *inode, struct file *filp) {
	module_put(THIS_MODULE);
    return 0;
}

ssize_t simple_read(struct file* filp, char* user_buf, size_t count, loff_t* f_pos) {
	int max, err, length;
	
	if(mountpoint == 0) {
		return -ENODATA;
	}
	
	length = strlen(mountpoint);
	max = length;
    if (max > count) {
		max = count;
	}
    
	/*
	 * If we're at the end of the message, 
	 * return 0 signifying end of file 
	 */
	if ((*f_pos) >= length)
		return 0;
    
    printk(KERN_INFO "SIMPLE: mountpoint %s %d\n", mountpoint, max);
    
	/* 
	 * Actually put the data into the buffer 
	 */
	err = copy_to_user(user_buf, mountpoint, max);
	if (err) {
		printk(KERN_WARNING "SIMPLE: error reading %d", err);
	}	
	(*f_pos)+=max;

	return max;
}

ssize_t simple_write(struct file* filp, const char* user_buf, size_t count, loff_t* f_pos) {
	struct path file_path; int result; 
    char* buf = kmalloc(count, GFP_KERNEL);
    strncpy(buf, user_buf, count-1);
    buf[count-1] = '\0';
    result =  kern_path(buf, 0, &file_path);
    kfree(buf);
    if (result != 0) {
		printk(KERN_WARNING "SIMPLE: data passed is not a file path.\n");
		return -EINVAL;
	}
	mountpoint = file_path.mnt->mnt_root->d_name.name;
	
	return count;
}

ssize_t simple_read_proc(struct file *filp, char *user_buf, size_t count, loff_t *f_pos) {
	return simple_read(filp, user_buf, count, f_pos);
}
