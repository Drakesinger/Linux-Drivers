/*
 * chardev.c: Creates a read-olny char device that says how many times
 * you've read from the dev file.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

/*
 * Prototype - this would normally go in a header file.
 */

int init_module(void);
void cleanup_module(void);
static int device_open(struct inode*, struct file*);
static int device_release(struct inode*, struct file*);
static ssize_t device_read(struct file*, char*, size_t, loff_t*);
static ssize_t device_write(struct file*, const char*, size_t, loff_t*);

// Definitions.
#define SUCCESS 0
#define DEVICE_NAME "chardev" // Device name as it appears in /proc/devices.
#define BUF_LEN 80 // Max length of the message FROM the device.

// Global variables are declared as static so they are global within the FILE.

static int Major;		// Major number assigned to our device driver.
static int Device_Open = 0;	// Used to prevent multiple access to the device.

static char msg[BUF_LEN]; 	// The message the device will return when asked.
static char* msg_Ptr;

static struct file_operations fops = {
	.read = device_read,
	.write = device_write,
	.open = device_open,
	.release = device_release
};

// Function called when the module is loaded.
int init_module(void)
{

	// Register the character module dynamically.
	// This is done by giving this function the Major 0.
	Major = register_chrdev(0, DEVICE_NAME, &fops);
	
	if (Major < 0)
	{
		printk(KERN_ALERT "Registering char device failed with %d\n", Major);
		return Major;
	}

	// All good, let's inform the reader of the logs.
	printk(KERN_INFO "I was assigned major number: %d. To talk to\n", Major);
	printk(KERN_INFO "the driver, create a dev fiile with\n");
	printk(KERN_INFO "mknod /dev/%s c %d 0.\n", DEVICE_NAME, Major);
	printk(KERN_INFO "Try various minor number. Try to cat and echo to\n");
	printk(KERN_INFO "the device file.\n");
	printk(KERN_INFO "Remove the device file and module when done.\n");

	return SUCCESS;
}

// This function gets called when the module is unloaded.
void cleanup_module()
{
	// Unregister the device.
	// this function return void!
	unregister_chrdev(Major, DEVICE_NAME);
}

// Module functionalities.

// Called when a process tries to open the device file like: "cat /dev/mycharfile"
static int device_open(struct inode* inode, struct file* filp)
{
	static int counter = 0;
	
	if (Device_Open)
	{
		return -EBUSY;
	}

	Device_Open++;
	sprintf(msg, "I already told you %d times Hello world!\n", counter++);
	msg_Ptr = msg;
	try_module_get(THIS_MODULE);

	return SUCCESS;
}




// Called when a process closes the device file.
static int device_release(struct inode* inode, struct file* file)
{
	Device_Open--; // We're now ready for our next caller.
	
	/*
	 * Decrement the usage count, or else once you opened the file, 
	 * you'll never get rid of the module.
	 */
	module_put(THIS_MODULE);
	return 0;
}

/*
 * Called when a process, that has already opened the device file,
 * attempts to read from it.
 */
static ssize_t device_read(struct file* filp,	/* see include/linux/fs.h */
			   char* buffer, 	/* buffer to fill with data */
			   size_t length, 	/* length of the buffer */
			   loff_t* offset)
{

	// Number of bytes actuallly written to the buffer.
	int bytes_read = 0;
	
	/*
	 * If we're at the end of the message:
	 * return 0, meaning EoF.
	 */
	if (*msg_Ptr == 0)
	{
		return 0;
	}

	// Put the data into the buffer.
	while (length && *msg_Ptr)
	{
		/*
		 * The buffer is in the user data segment, not the kernel segment
		 * so "*" assignment won't work. We have to use put_user
		 * which copies data from the kernel's data segment to
		 * the user's data segment.
		 */
		put_user(*(msg_Ptr++), buffer++);
		
		length--;
		bytes_read++;
	}

	// Most read functions return the number of bytes put into the buffer.
	return bytes_read;
}


// Called when a process writes to dev file: echo "hi" > /dev/hello
static ssize_t
device_write(struct file* filp, const char* buff, size_t len, loff_t* off)
{
	printk(KERN_ALERT "Sorry, this operation isn't supported. Yet.\n");
	return -EINVAL;
}
