#include "double_module.h"

#define mybuf_size 100
#define double_major 200

const char *text = "double. Read calls: %d, Write calls: %d\n";

int read_count = 0;
int write_count = 0;

char* mybuf = 0;	/* The messages the device will give when asked */
int mybuf_count = 0;

struct proc_dir_entry *proc_entry = 0;

int __init double_init(void) {
    int result;

    /* Register an entry in /proc */
    proc_entry = proc_create("double", 0, NULL, &proc_fops);

    /* Register a device with the given major number */
    result = register_chrdev(double_major, "double", &double_fops);
    if (result < 0) {
        printk(KERN_WARNING "Cannot register the /dev/double device with major number: %d\n", double_major);
        return result;
    }

    mybuf = kmalloc(mybuf_size, GFP_KERNEL);
    if (!mybuf) {
        result = -ENOMEM;
        double_exit();
        return result;
    } else {
        memset(mybuf, 0, 1); // not important
        printk(KERN_INFO "The double module has been inserted.\n");
        return 0;
    }
}

void double_exit(void) {
    /* Unregister the device and /proc entry */
    unregister_chrdev(double_major, "double");
	if (proc_entry) {
		proc_remove(proc_entry);
	}

    /* Free the buffer */
    if (mybuf) {
        kfree(mybuf);
    }

    printk(KERN_INFO "The double module has been removed\n");
}

int double_open(struct inode *inode, struct file *filp) {
	try_module_get(THIS_MODULE);
    return 0;
}

int double_release(struct inode *inode, struct file *filp) {
	module_put(THIS_MODULE);
    return 0;
}

ssize_t double_read(struct file *filp, char *user_buf, size_t length, loff_t *f_pos) {
    /* Move one byte to the userspace */
    // copy_to_user(void *to, void* from, int size)

	int bytes_left = mybuf_count - (*f_pos);
	int max = bytes_left;
    int err;
    if (max > length) {
		max = length;
	}

    read_count++;
    
	/*
	 * If we're at the end of the message, 
	 * return 0 signifying end of file 
	 */
	if (bytes_left <= 0)
		return 0;
    
	/* 
	 * Actually put the data into the buffer 
	 */
	
	err = copy_to_user(user_buf, mybuf, max);
	if (err) {
		printk(KERN_WARNING "double: error occured in double_read: %d", err);
	}	
	(*f_pos)+=max;

	/* 
	 * Most read functions return the number of bytes put into the buffer
	 */
	return max;
}

ssize_t double_write(struct file *filp, const char *user_buf, size_t count, loff_t *f_pos) {
    // copy_from_user(to, from, size)
    // appending
   
	int max = count;
    int err;
    if (max > (mybuf_size - mybuf_count))
		max = mybuf_size - mybuf_count;
	// already written
	printk(KERN_INFO "mybuf_count value: %d\n", (int) mybuf_count);
	printk(KERN_INFO "max value: %d\n", (int) max);
	printk(KERN_INFO "mybuf_size value: %d\n", (int) mybuf_size);
	
	if(max > 0) {
		err = copy_from_user(&mybuf[mybuf_count], user_buf, max);
		if (err) {
			printk(KERN_WARNING "double: error occured in double_write: %d", err);
		}	
		mybuf_count+=max;
		//mybuf[mybuf_count] = 0;
		
		write_count++;
		return max;
	} 
	
    return -ENOSPC;
}

bool copied = false;

ssize_t double_read_proc(struct file *filp, char *user_buf, size_t count, loff_t *f_pos) {
	char *buf;
	int length;
	int err;

	buf = kmalloc(1000, GFP_KERNEL);
	if (!buf) goto out;

	if (!copied) {
		length = snprintf(buf, 1000, text, read_count, write_count);
		if (count >= length) {
			count = length;
		}

		err = copy_to_user(user_buf, buf, count);
		if (err) {
	    	printk(KERN_WARNING "double: error occured in double_read_proc: %d\n", err);
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
