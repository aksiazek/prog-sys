#include "simple_module.h"

#define simple_major 199

struct proc_dir_entry* proc_entry = 0;
uid_t owner = -1;
bool copied = false;

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
		printk(KERN_INFO "The SIMPLE proc has been removed\n");
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
	char* buf;
	int length;
	int err;
	
	if((int) owner == -1) {
		return -ENODATA;
	}

	/* max uid is 2^16, so five characters plus 8 in msg*/
	buf = kmalloc(13, GFP_KERNEL);
	if (!buf) goto out;

	if (!copied) {
		length = snprintf(buf, 13, "owner: %d\n", (int) owner);
		if (count >= length) {
			count = length;
		}

		err = copy_to_user(user_buf, buf, count);
		if (err) {
	    	printk(KERN_WARNING "SIMPLE: error occured in simple_read_proc: %d\n", err);
			goto out;
		}
		copied = true;
	} else {
    	count = 0;
		copied = false;
	}

out:
	if (buf) {
		kfree(buf);
	}
	return count;
}

ssize_t simple_write(struct file* filp, const char* user_buf, size_t count, loff_t* f_pos) {
    struct path file_path; int result; struct kstat kstat_local;
    char* buf = kmalloc(count, GFP_KERNEL);
    strncpy(buf, user_buf, count-1);
    buf[count-1] = '\0';
    result =  kern_path(buf, 0, &file_path);
    kfree(buf);
    if (result != 0) {
		printk(KERN_WARNING "SIMPLE: data passed is not a file path.\n");
		return -EINVAL;
	}
	
	result =  vfs_getattr(&file_path, &kstat_local);
	if (result != 0) {
		printk(KERN_WARNING "SIMPLE: error in vfs_getattr\n");
		return -EINVAL;
	}
	
	owner = from_kuid(&init_user_ns, kstat_local.uid);
	
	return count;
}

ssize_t simple_read_proc(struct file *filp, char *user_buf, size_t count, loff_t *f_pos) {
	return simple_read(filp, user_buf, count, f_pos);
}
