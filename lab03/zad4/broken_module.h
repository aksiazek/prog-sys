#include <linux/init.h>
//#include <linux/config.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/proc_fs.h>
#include <linux/fcntl.h>
#include <linux/ctype.h>
#include <asm/system.h>
#include <asm/uaccess.h>

MODULE_LICENSE("GPL");

int broken_open(struct inode *inode, struct file *filp);
int broken_release(struct inode *inode, struct file *filp);
ssize_t broken_read(struct file *filp, char *user_buf, size_t count, loff_t *f_pos);
ssize_t broken_write(struct file *filp, const char *user_buf, size_t count, loff_t *f_pos);
int broken_read_proc(char *page, char **start, off_t offset, int count, int *eof, void *data);
void broken_exit(void);
int broken_init(void);

/* Struktura ktora opisuje funkcje uzywane przy dostepie do /dev/broken */
struct file_operations broken_fops = {
    read: broken_read,
    write: broken_write,
    open: broken_open,
    release: broken_release
};

/* Deklaracje standardowych funkcje inicjalizacji i wyjscia modulu */
module_init(broken_init);
module_exit(broken_exit);

/* Zmienne globalne */
int broken_major = 899;
int read_count = 0;
int write_count = 0;
