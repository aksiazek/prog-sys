#include <linux/linkage.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/hashtable.h>
#include <linux/slab.h>

DEFINE_HASHTABLE(hashmap, 4);

struct user_hash {
	struct hlist_node hash;
	unsigned int uid;
	unsigned int count;
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
	unsigned int bkt, max, uid = 0;

	rcu_read_lock();

    for_each_process(task) {
		entry = kmalloc(sizeof(struct user_hash), GFP_KERNEL);
		entry->uid = task->real_cred->uid.val;
		entry->count = 1;
		
		//pr_info("%s [%d] [%d]\n", task->comm, task->pid, entry->uid);
		existing = get(entry->uid);
		if(existing != NULL) {
			existing->count++;
			//printk(KERN_INFO "%u %u", existing->uid, existing->count);
		}
		else {
			hash_add(hashmap, &entry->hash, entry->uid);
			//printk(KERN_INFO "first time %u", entry->uid);
		}
		
	}
	
	rcu_read_unlock();
	
	max = 0;
	hash_for_each(hashmap, bkt, existing, hash) 
	{
		if(existing->count > max)
		{
			max = existing->count;
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
	
	if(max == 0)
		return EPERM;
    return uid;
}
