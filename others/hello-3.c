/*
 * hello-3.c - Illustrating the __init, __initdata and __exit macros.
 */

#include <linux/module.h>	/* Needed by all modules.*/
#include <linux/kernel.h>	/* Needed for KERN_INFO. */
#include <linux/init.h> 	/* Needed for the macros. */

static int hello3_data __initdata = 3;

static int __init hello_3_init(void)
{
	// KERN_INFO is the log channel used. There are 8 channels.
	printk(KERN_INFO "Hello world %d.\n", hello3_data);
	
	/*
	 * A non 0 return means init_module failed; module can't be loaded.
	 */
	return 0;
}

static void __exit hello_3_exit(void)
{
	printk(KERN_INFO "Goodbye world 3.\n");
}

// Define the macros here and choose which functions to use.
module_init(hello_3_init);
module_exit(hello_3_exit);
