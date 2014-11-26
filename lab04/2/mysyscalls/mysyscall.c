#include <linux/linkage.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/rcupdate.h>

asmlinkage long sys_mysyscall(char* user_buf, long which)
{
    //printk (KERN_EMERG "Hura! Moge napisac 'Hello world' z wnetrza jadra!\n");
	struct task_struct* task;
	char* buf;
	long num = 0;

	rcu_read_lock();
	//for_each_process(task)
	//	printk(KERN_EMERG  "%ld", task->state);

	task = &init_task;
	while((num < which) && (next_task(task) != &init_task)) {
        //pr_info("%s [%d] [%d]\n", task->comm, task->pid, task->real_cred->uid);
        task = next_task(task);
		num++;
	}
	num = strlen(task->comm);
	if((next_task(task) != &init_task) || (which == 0)) {
		buf = kmalloc(16, GFP_KERNEL);
		buf = strncpy(buf, task->comm, num);
		buf[num] = 0;
		copy_to_user(user_buf, buf, num+1);
		kfree(buf);
	} else
		return 3;
	
	rcu_read_unlock();
    return 0;
}
