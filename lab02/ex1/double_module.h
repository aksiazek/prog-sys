#include <linux/module.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/proc_fs.h>
#include <linux/fcntl.h>
#include <asm/uaccess.h>

MODULE_LICENSE("GPL");

int double_open(struct inode *inode, struct file *filp);
int double_release(struct inode *inode, struct file *filp);
ssize_t double_read(struct file *filp, char *user_buf, size_t count, loff_t *f_pos);
ssize_t double_write(struct file *filp, const char *user_buf, size_t count, loff_t *f_pos);
ssize_t double_read_proc(struct file *filp, char *user_buf, size_t count, loff_t *f_pos);
void double_exit(void);
int double_init(void);

/* Operations for /dev/double */
struct file_operations double_fops = {
    .read = double_read,
    .write = double_write,
    .open = double_open,
    .release = double_release
};

/* Operations for /proc/double */
struct file_operations proc_fops = {
	.read = double_read_proc
};

module_init(double_init);
module_exit(double_exit);
