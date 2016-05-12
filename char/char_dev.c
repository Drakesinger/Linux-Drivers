/*
 * char_dev.c: Creates a read-olny char device that says how many times
 * you've read from the dev file.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Horia Mut <horiamut@msn.com>");
MODULE_DESCRIPTION("A kernel module developed for the Sys DEV course.");

/*
 * Prototype - this would normally go in a header file.
 */

int init_module(void);
void cleanup_module(void);
static int device_open(struct inode*, struct file*);
static int device_release(struct inode*, struct file*);
static ssize_t device_read(struct file*, char*, size_t, loff_t*);
static ssize_t device_write(struct file*, const char __user*, size_t, loff_t*);

// Definitions.
#define SUCCESS 0
#define DEVICE_NAME "chardev" // Device name as it appears in /proc/devices.
#define BUF_LEN 80 // Max length of the message FROM the device.

// Global variables are declared as static so they are global within the FILE.

static int Major;		// Major number assigned to our device driver.
static int Device_Open = 0;	// Used to prevent multiple access to the device.

static char msg_read[BUF_LEN]; 	// The message the device will return when asked.
static char msg_write[BUF_LEN]; // The message the device will return when writing.
static char* msg_read_Ptr;
static char* msg_write_Ptr;

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
	printk(KERN_INFO "Device %s unregistered.\n", DEVICE_NAME);
}

// Module functionalities.

// Called when a process tries to open the device file like: "cat /dev/mycharfile".
static int device_open(struct inode* inode, struct file* filp)
{
	static int counter = 0;
	
	if (Device_Open)
	{
		return -EBUSY;
	}

	Device_Open++;
	// Don't do this for now.
	//sprintf(msg_read, "I already told you %d times Hello world!\n", counter++);
  	printk(KERN_INFO "device_open, msg_read = %s, msg_write = %s\n", msg_read, msg_write);	
	// Make our pointers point to the correct place.
	msg_read_Ptr = msg_read;
	msg_write_Ptr = msg_write;

	try_module_get(THIS_MODULE);

	return SUCCESS;
}


// Called when a process closes the device file.
static int device_release(struct inode* inode, struct file* file)
{
	printk(KERN_INFO "device_release\n");
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
	if (*msg_read_Ptr == 0)
	{
		printk(KERN_INFO "read. EOF!");
		return 0;
	}

	printk("read put: *msg_read_Ptr = %c\n", *msg_read_Ptr);

	// Put the data into the buffer.
	while (length && *msg_read_Ptr)
	{
		/*
		 * The buffer is in the user data segment, not the kernel segment
		 * so "*" assignment won't work. We have to use put_user
		 * which copies data from the kernel's data segment to
		 * the user's data segment.
		 *
		 * So this copies from msg_read_Ptr to buffer.
		 */
		put_user(*(msg_read_Ptr++), buffer++);
		
		length--;
		bytes_read++;
	}

	// More debug.
	printk(KERN_INFO "device_read: %d bytes, %d left\n", bytes_read, length);

	// Most read functions return the number of bytes put into the buffer.
	return bytes_read;
}


// Called when a process writes to dev file: echo "hi" > /dev/hello
static ssize_t
device_write(struct file* filp, const char __user*  buff, size_t len, loff_t* off)
{
	int i;
       	int j;	
	int bytes_written = 0;
	//printk(KERN_INFO "device_write(%p, %s)\n", filp, buff);
	
	// Get message from user data segment.
	for(i = 0; i < len && i < BUF_LEN; i++)
	{
		// Fill the buffer from the user data segment.
		// copy from the pointer buff to msg.
		get_user(msg_write[i], buff + i);	
	}
	
	bytes_written = i;
	i -= 2; // Take out the "\n\0".
	
	// msg_write now contains the buffer.
	// We can reverse it if we want.
	for(j = 0; j <= bytes_written && j < BUF_LEN && i >= 0; j++)
	{
		// Problem may be the finishing 0.
		if(msg_write[i] == '\0' || msg_write[i] == '\n')
		{
			printk(KERN_INFO "Should not happen.\n");
			msg_read[j] = '0';
			i--;
		}
		else
		{
			msg_read[j] = msg_write[i--];
		}
		//printk(KERN_INFO "msg_read[%d]=%c\n",j,msg_read[j]);
		// This confirms that the array is filled.
	}

	// Add the /n and /0
	msg_read[++j] = msg_write[bytes_written-1];	//'\n';
	msg_read[++j] = msg_write[bytes_written];	//'\0';

	printk(KERN_INFO "device_write, msg_read = %s, msg_write = %s\n", msg_read, msg_write);
	printk(KERN_INFO "bytes_written : %d\n", bytes_written);
	
	// This is the part where everything goes to hell.
	// Doesn't show anything in console.
	//sprintf(msg, msg_write);

	// This is a bit weird. Should have already been done.
	msg_read_Ptr = msg_read;
	msg_write_Ptr = msg_write;

	return bytes_written;
}
