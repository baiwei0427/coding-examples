#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/random.h>

int init_module(void)
{
        unsigned int i, random;

        for (i = 0; i < 10; i++) {
                get_random_bytes(&random, sizeof(random));
                random = random % 100;
                printk(KERN_INFO "%u\n", random);
        }

        return 0;
}


void cleanup_module(void)
{
        printk(KERN_ALERT "Goodbye world 1.\n");
}

MODULE_LICENSE("GPL");
