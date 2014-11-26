#include <linux/linkage.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/namei.h>
#include <linux/statfs.h>
#include <linux/fs.h>

asmlinkage long long sys_mysyscall(char* user_buf, unsigned int length)
{
	struct path file_path;
	struct kstatfs kstat;
	int result;
	char* buf;
	long long ret_val;
	
	if(!(length > 0)) return -EINVAL;
	
	buf = kmalloc(length+1, GFP_KERNEL); // longer file paths are not supported	
	
	copy_from_user(buf, user_buf, length);
	buf[length] = '\0';
	printk(KERN_INFO "buf is %s\n", buf);
	
	result = kern_path(buf, 0, &file_path);
	kfree(buf);
	
	if (result != 0) {
		printk(KERN_WARNING "data passed is not a file path.\n");
		return -EINVAL;
	}
	
	vfs_statfs(&file_path, &kstat); 
	printk(KERN_INFO "d\n");
	
	ret_val = kstat.f_bfree;
    return ret_val;
}
