/*
 * hello-4.c - Demonstrates module documentation.
 */

#include <linux/module.h>	/* Needed by all modules.*/
#include <linux/kernel.h>	/* Needed for KERN_INFO. */
#include <linux/init.h> 	/* Needed for the macros. */

// The module definitions come here.
#define DRIVER_AUTHOR 	"Horia Mut <horiamut@msn.com>"
#define DRIVER_DESC	"A sample driver"

static int hello4_data __initdata = 4;

static int __init hello_4_init(void)
{
	// KERN_INFO is the log channel used. There are 8 channels.
	printk(KERN_INFO "Hello world %d.\n", hello4_data);
	
	/*
	 * A non 0 return means init_module failed; module can't be loaded.
	 */
	return 0;
}

static void __exit hello_4_exit(void)
{
	printk(KERN_INFO "Goodbye world 4.\n");
}

// Define the macros here and choose which functions to use.
module_init(hello_4_init);
module_exit(hello_4_exit);

/*
 * You can use strings, like this:
 */

/*
 * Get rid of taint message by declaring the code as GPL.
 */
MODULE_LICENSE("GPL");

/*
 * Or with defines, like this:
 */
MODULE_AUTHOR(DRIVER_AUTHOR);		// Who wrote this module.
MODULE_DESCRIPTION(DRIVER_DESC);	// What does the module do?

/*
 * This module uses /dev/testdevice. The MODULE_SUPPORTED_DEVICE macro might
 * be used in the future to help automatic configuration of modules, but it is
 * currently unused other than for documentationn purposes.
 * 
 * This was the case in 2001.
 */
MODULE_SUPPORTED_DEVICE("testdevice");
