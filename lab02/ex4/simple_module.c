#include "simple_module.h"

#define simple_major 199

struct proc_dir_entry *proc_entry = 0;
const char* mountpoint = 0;

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
