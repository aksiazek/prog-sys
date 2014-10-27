#include "broken_module.h"

char *mybuf=NULL;
int mybuf_size=100;

int fill_buffer(char* buf, int buf_size) {
    sprintf(buf, "I've created a buffer of size: %d\n", buf_size);
    return strlen(buf);
}

int broken_init(void) {
    int result;

    /* Rejestracja pliku w systemie proc */
    create_proc_read_entry("broken", 0, NULL, broken_read_proc, NULL);

    /* Rejestracja urzadzenia */
    result = register_chrdev(broken_major, "broken", &broken_fops);
    if (result < 0) {
        printk("<1>Cannot register the /dev/broken device with major number: %d\n", broken_major);
        return result;
    }

    printk("<1>The BROKEN module has been inserted.\n");
    return 0;
}


void broken_exit(void) {
    /* Odrejestrowanie urzadzenia */
    unregister_chrdev(broken_major, "broken");

    printk("<1>The BROKEN module has been removed\n");
}

int broken_open(struct inode *inode, struct file *filp) {
    return 0;
}

int broken_release(struct inode *inode, struct file *filp) {
    return 0;
}

ssize_t broken_read(struct file *filp, char *user_buf, size_t count, loff_t *f_pos) {
   char* buf = NULL;
   int buf_size = 100;
   int len, err;

    buf = kmalloc(buf_size, GFP_KERNEL);
    if (buf == 0) {
       broken_exit();
       return -ENOMEM;
    } else {
       fill_buffer(buf,buf_size);
    }
   
    len = strlen(buf);
    err = copy_to_user(user_buf, buf, len);
    kfree(buf);
    
    read_count++;


    if (!err && *f_pos == 0) {
      *f_pos += len;
      return len;
    } else {
      return 0;
    }
}

ssize_t broken_write(struct file *filp, const char *user_buf, size_t count, loff_t *f_pos) {
    return 1;
}

int broken_read_proc(char *page, char **start, off_t offset, int count, int *eof, void *data) 
{
    return sprintf(page, "BROKEN. Read calls: %d, Write calls: %d\n", read_count, write_count);
}
