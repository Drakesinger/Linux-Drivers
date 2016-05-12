/*
 * stop.c - Illustration of multi filed modules.
 */

#include <linux/module.h> /* Needed by all modules.*/
#include <linux/kernel.h> /* Needed for KERN_INFO. */


void cleanup_module(void)
{
        printk(KERN_INFO "Goodbye world, the kernel module has a short life.\n");
}
