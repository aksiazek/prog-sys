#include <linux/linkage.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/rcupdate.h>

asmlinkage long sys_mysyscall(char* user_buf, unsigned int pid)
{
	struct task_struct* task;
	char* buf;
	long num = 0;

	rcu_read_lock();
	//for_each_process(task)
	//	printk(KERN_EMERG  "%ld", task->state);

	task = &init_task;
	while((task->pid != pid) && (next_task(task) != &init_task))
		task = next_task(task);
	
	num = strlen(task->comm);
	if(task->pid == pid) {
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
