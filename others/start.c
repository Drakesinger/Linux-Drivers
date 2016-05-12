/*
 * start.c - Illustration of multi filed modules.
 */

#include <linux/module.h> /* Needed by all modules.*/
#include <linux/kernel.h> /* Needed for KERN_INFO. */

int init_module(void)
{
	printk(KERN_INFO "Hello world, kernel speaking.\n");
	
	/*
	 * A non 0 return means init_module failed; module can't be loaded.
	 */
	return 0;
}
