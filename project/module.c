#include "module.h"

module_init(stats_of_random_init);
module_exit(stats_of_random_exit);

/*  /proc/random entries */
struct proc_dir_entry *parent_dir = NULL;
struct proc_dir_entry *bytes_read_proc = NULL;
struct proc_dir_entry *readers_proc = NULL;

struct file_operations bytes_read_proc_fops = {
	.read = all_bytes_read_proc
};

struct file_operations readers_proc_fops = {
	.read = all_readers_proc
};

/* atomic ? */
unsigned long all_bytes = 0;
unsigned long reader_count = 0; 

struct user_hash {
	struct hlist_node hash;
	unsigned int pid;
	char* reader_name; // comm
	unsigned long bytes_read;
	bool finished;
};

DEFINE_HASHTABLE(hashmap, 4);

static struct user_hash* get(unsigned int pid)
{
	struct user_hash* uh_slot;
	hash_for_each_possible_rcu(hashmap, uh_slot, hash, pid)
	{
		if (pid == uh_slot->pid)
			return uh_slot;
	}
	return NULL;
}

bool eof = false;

ssize_t all_bytes_read_proc(struct file *filp, char *user_buf, size_t count, loff_t *f_pos) {
	char *buf;
	int length, err;
	
	buf = kmalloc(100, GFP_KERNEL);
	
	if (!buf)
		goto out;
	
	if (!eof) {
		length = snprintf(buf, 100, "%lu\n", all_bytes);
		if (count >= length) {
			count = length;
		}
		err = copy_to_user(user_buf, buf, count);
		if (err) {
			goto out;
		}
		eof = true;
		
		
	} else {
		count = 0;
		eof = false;
		
	}
	
	out:
	if (buf) {
		kfree(buf);
	}
	return count;
}

ssize_t all_readers_proc(struct file *filp, char *user_buf, size_t count, loff_t *f_pos) {
	char *buf;
	int length, err;
	
	buf = kmalloc(100, GFP_KERNEL);
	
	if (!buf)
		goto out;
	
	if (!eof) {
		length = snprintf(buf, 100, "%lu\n", all_bytes);
		if (count >= length) {
			count = length;
		}
		err = copy_to_user(user_buf, buf, count);
		if (err) {
			goto out;
		}
		eof = true;
		
		
	} else {
		count = 0;
		eof = false;
		
	}
	
	out:
	if (buf) {
		kfree(buf);
	}
	return count;
}

struct kernsym sym_random_read;

struct symhook random_read_hook = {
		"random_read", 
		&sym_random_read, 
		(unsigned long *)random_read,
};

ssize_t random_read(struct file *file, char __user *buf, size_t nbytes, loff_t *ppos)
{
	struct user_hash* existing;
	struct user_hash* entry;
	ssize_t bytes_read;
	ssize_t (*run)(struct file*, char*, size_t, loff_t*);
	
	existing = get(current->pid); // can be the same process or one with the same pid
	
	// pr_info("%s [%d]\n", current->comm, current->pid);
	
	if(existing == NULL) {
		reader_count++;
		entry = kmalloc(sizeof(struct user_hash), GFP_KERNEL);
		entry->pid = current->pid;
		entry->reader_name = current->comm;
		entry->bytes_read = 0;
		entry->finished = 0;
		hash_add_rcu(hashmap, &entry->hash, entry->pid);
		existing = entry;
	} else if (existing->finished) {
		// reuse
		reader_count++;
		existing->reader_name = current->comm;
		existing->bytes_read = 0;
		existing->finished = 0;
	}
	
	run = sym_random_read.run;
	bytes_read = run(file, buf, nbytes, ppos);
	
	all_bytes += bytes_read;
	existing->bytes_read += bytes_read;
	
	printk(KERN_INFO "bytes read: %lu %lu\n", (unsigned long) bytes_read, reader_count);
	
	if(bytes_read < 1) {
		reader_count--;
		// no deletion, keep record
	}
	
	return bytes_read;
}

int stats_of_random_init(void) {
    int ret;

	ret = kernfunc_init();
	if (IN_ERR(ret))
		return ret;

    /* Register an entry in /proc/random/ */
    parent_dir = proc_mkdir("random", NULL);
    bytes_read_proc = proc_create("bytes_read", 0, parent_dir, &bytes_read_proc_fops);
    readers_proc = proc_create("readers", 0, parent_dir, &readers_proc_fops);
    
    ret = symbol_hijack(random_read_hook.sym, random_read_hook.name, random_read_hook.func);
	if (IN_ERR(ret))
		printk(KERN_ALERT "%s ERROR\n", random_read_hook.name);
		
	printk(KERN_INFO "The stats_of_random module has been inserted.\n");
	return 0;
}

void stats_of_random_exit(void) {
	struct user_hash *existing, *entry;
	struct hlist_node* prev;
	unsigned long long bkt;
	
	if (bytes_read_proc) {
		proc_remove(bytes_read_proc);
	}
	if (readers_proc) {
		proc_remove(readers_proc);
	}
	if (parent_dir) {
		proc_remove(parent_dir);
	}
	
	symbol_restore(random_read_hook.sym);
	
	// hashmap cleanup!
	entry = NULL; // kfree on NULL is safe
	prev = NULL;
	hash_for_each(hashmap, bkt, existing, hash) {
		if(prev != NULL) hash_del(prev);
		kfree(entry);
		prev = &existing->hash;
		entry = existing;
	}
	if(prev != NULL) hash_del(prev);
	kfree(entry);

    printk(KERN_INFO "The stats_of_random module has been removed\n");
}


