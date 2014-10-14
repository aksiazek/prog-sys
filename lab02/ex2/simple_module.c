#include "simple_module.h"

#define mybuf_size 100
#define simple_major 199

struct proc_dir_entry *proc_entry = 0;
char* task_name = 0;

int __init simple_init(void) {
    int result;

    /* Register an entry in /proc */
    proc_entry = proc_create("simple", 0, NULL, &proc_fops);

    /* Register a device with the given major number */
    result = register_chrdev(simple_major, "simple", &simple_fops);
    if (result < 0) {
        printk(KERN_WARNING "Cannot register the /dev/simple device with major number: %d\n", simple_major);
        proc_remove(proc_entry);
        return result;
    }
    
	printk(KERN_INFO "The SIMPLE module has been inserted.\n");
	return 0;
}

void simple_exit(void) {
    /* Unregister the device and /proc entry */
    unregister_chrdev(simple_major, "simple");
	if (proc_entry) {
		proc_remove(proc_entry);
	}
	
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
	int max = 16; // comm field length
    int err;
	
	if(task_name == 0) {
		return -ENODATA;
	}
	
    if (max > count) {
		max = count;
	}
    
	/*
	 * If we're at the end of the message, 
	 * return 0 signifying end of file 
	 */
	if ((*f_pos) >= 16)
		return 0;
    
	/* 
	 * Actually put the data into the buffer 
	 */
	
	err = copy_to_user(user_buf, task_name, max);
	if (err) {
		printk(KERN_WARNING "SIMPLE: error reading %d", err);
	}	
	(*f_pos)+=max;

	return max;
}

ssize_t simple_write(struct file* filp, const char* user_buf, size_t count, loff_t* f_pos) {
    
    pid_t pid; int result; struct task_struct* task;
    char* buf = kmalloc(count+1, GFP_KERNEL);
    strncpy(buf, user_buf, count);
    buf[count] = '\0';
     
    result = kstrtouint(buf, 10, (unsigned int*) &pid);
    kfree(buf);
    
    if (result != 0) {
		printk(KERN_WARNING "SIMPLE: data passed is not a valid pid.\n");
		return -EINVAL;
	}
	
	task = pid_task(find_vpid(pid), PIDTYPE_PID);
	if(task == NULL) {
		printk(KERN_WARNING "SIMPLE: data passed is not a valid pid.\n");
		return -EINVAL;
	}
	task_name = task->comm;
	printk(KERN_INFO "SIMPLE: pid_t number to call is %d\n", (unsigned int) pid);
	
	return count;
}

ssize_t simple_read_proc(struct file *filp, char *user_buf, size_t count, loff_t *f_pos) {
	return simple_read(filp, user_buf, count, f_pos);
}
