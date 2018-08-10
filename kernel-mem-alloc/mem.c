#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gfp.h>
#include <linux/mm_types.h>
#include <asm/pgtable_64.h>
#include <linux/mmzone.h>

// Allocate 2 ^ order pages
static unsigned int order = MAX_ORDER - 1;
// Memory pages
static struct page *pages = NULL;

// Allocate physical contigious memory
static void alloc_memory()
{       
        unsigned int i, num_pages = 1 << order;
        unsigned long phy_addr;
        pages = alloc_pages(GFP_KERNEL, order);

        if (!pages) {
                printk(KERN_INFO "Fail to allocate %u pages\n", num_pages);
                return;
        }
        
        printk(KERN_INFO "Successfully allocate %u pages\n", num_pages);                
        
        for (i = 0; i < num_pages; i++) {
                phy_addr = page_to_phys(&pages[i]);
                printk(KERN_INFO "Physical address of page %u: %lx\n", i, phy_addr);
        }
}

// Free the memory
static void free_memory()
{
        if (!pages) {
                printk(KERN_INFO "No page to free\n");
                return;                
        }

        __free_pages(pages, order);
        printk(KERN_INFO "Free %u pages\n", 1 << order);
}

int init_module(void)
{
        printk(KERN_INFO "Install kernel module\n");
        alloc_memory();
        return 0;
}


void cleanup_module(void)
{       
        free_memory();
        printk(KERN_INFO "Remove kernel module\n");
}

MODULE_LICENSE("GPL");
