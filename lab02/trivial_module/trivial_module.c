#include <linux/module.h>

MODULE_LICENSE("GPL");

static int __init trivial_init(void) {
    printk(KERN_INFO "Hello world! I'm a trivial module!\n");
    return 0;
}

static void __exit trivial_exit(void) {
    printk(KERN_INFO "Oh no, why are you doing this to me? Argh!\n");
}

module_init(trivial_init);
module_exit(trivial_exit);

