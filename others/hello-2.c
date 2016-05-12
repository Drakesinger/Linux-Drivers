/*
 * hello-2.c - Demonstrating the module_init() and module_exit() macros.
 * This is the preferred over using init_module() and cleanup_module().
 */

#include <linux/module.h>	/* Needed by all modules.*/
#include <linux/kernel.h>	/* Needed for KERN_INFO. */
#include <linux/init.h> 	/* Needed for the macros. */

static int __init hello_2_init(void)
{
	// KERN_INFO is the log channel used. There are 8 channels.
	printk(KERN_INFO "Hello world 2.\n");
	
	/*
	 * A non 0 return means init_module failed; module can't be loaded.
	 */
	return 0;
}

static void __exit hello_2_exit(void)
{
	printk(KERN_INFO "Goodbye world 2.\n");
}

// Define the macros here and choose which functions to use.
module_init(hello_2_init);
module_exit(hello_2_exit);
