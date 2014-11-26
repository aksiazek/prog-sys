#include <linux/linkage.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/hashtable.h>
#include <linux/slab.h>

DEFINE_HASHTABLE(hashmap, 4);

struct user_hash {
	struct hlist_node hash;
	unsigned int uid;
	unsigned long long total_cpu_time;
};

static struct user_hash* get(unsigned int uid)
{
	struct user_hash* uh_slot;
	hash_for_each_possible(hashmap, uh_slot, hash, uid)
	{
		if (uid == uh_slot->uid)
			 return uh_slot;
	}
    return NULL;
}

asmlinkage unsigned int sys_mysyscall(void)
{
	struct task_struct* task;
	struct user_hash* existing;
	struct user_hash* entry;
	struct hlist_node* prev;
	unsigned long long bkt, max;
	unsigned int uid = 0;

	rcu_read_lock();

    for_each_process(task) {
		entry = kmalloc(sizeof(struct user_hash), GFP_KERNEL);
		entry->uid = task->real_cred->uid.val;
		entry->total_cpu_time = task->utime + task->stime;
		
		pr_info("%s [%d] [%d]\n", task->comm, task->pid, entry->uid);
		existing = get(entry->uid);
		if(existing != NULL) {
			existing->total_cpu_time += task->utime + task->stime;
			printk(KERN_INFO "%u %llu", existing->uid, existing->total_cpu_time);
		}
		else {
			hash_add(hashmap, &entry->hash, entry->uid);
			printk(KERN_INFO "first entry %u %llu", entry->uid, entry->total_cpu_time);
		}
		
	}
	
	rcu_read_unlock();
	
	max = 0;
	uid = 0;
	hash_for_each(hashmap, bkt, existing, hash) 
	{
		if(existing->total_cpu_time > max)
		{
			max = existing->total_cpu_time;
			uid = existing->uid;
		}
		
	}
	
	entry = NULL; // kfree on NULL is safe
	prev = NULL;
	// cleanup
	hash_for_each(hashmap, bkt, existing, hash) {
		if(prev != NULL) hash_del(prev);
		kfree(entry);
		prev = &existing->hash;
		entry = existing;	
	}
	hash_del(prev);
	kfree(entry);
	
    return uid;
}
