/*
 * char_dev.c: Create an input/output character device
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

// Prototypes header.
#include "char_dev.h"


// Definitions.
#define SUCCESS 0
#define DEVICE_NAME "char_dev" // Device name as it appears in /proc/devices.
#define BUF_LEN 80 // Max length of the message FROM the device.

// Global variables are declared as static so they are global within the FILE

// Used to prevent multiple access to the device.
static int Device_Open = 0;

// The message the device will return when asked.
static char Message[BUF_LEN];
static char* Message_Ptr;



static int device_open(struct inode* inode, struct file* file)
{
	printk(KERN_INFO "device_open(%p, %p)\n", inode, file);
	
	if (Device_Open)
	{
		return -EBUSY;
	}

	Device_Open++;
	// Initialize the message.
	Message_Ptr = Message;
	try_module_get(THIS_MODULE);

	return SUCCESS;
}


static int device_release(struct inode* inode, struct file* file)
{
	printk(KERN_INFO "device_release(%p, %p)\n", inode, file);

	// Ready for our next caller.
	Device_Open--;

	module_put(THIS_MODULE);
	return SUCCESS;
}

/**
 * @param buffer	User space buffer to be filled with data.
 */
static ssize_t device_read(struct file* file, char __user* buffer, size_t length, loff_t offset)
{
	// Number of bytes actually written to the buffer.
	int bytes_read = 0;

	printk(KERN_INFO "device_read(%p, %p, %d)\n", file, buffer, length);

	// If we're at the end of the message, return 0.
	if (*Message_Ptr == 0)
	{
		return 0;
	}

	// Put the data into the buffer.
	while( length && *Message_Ptr)
	{
		// Use put_user since the buffer is in user data segement
		// and not the kernel data segment.
		put_user(*(Message_Ptr++), buffer++);
		length--;
		bytes_read++;
	}

	// Print more debugging information.
	printk(KERN_INFO "Read %d bytes, %d left\n", bytes_read, length);

	// Read functions normally return the number of bytes inserted
	// into the buffer.
	return bytes_read;	
}


static ssize_t device_write(struct file* file, const char __user* buffer, size_t length, loff_t* offset)
{
	int i;

	printk(KERN_INFO "device_write(%p, %s, %d)", file, buffer, length);
	
	// Get the message from user data segment.
	for(i = 0; i < length && i < BUF_LEN; i++)
	{
		get_user(Message[i], buffer + i);
	}

	// Set the pointer to point to the message written.
	Message_Ptr = Message;

	// Return the number of input characters used.
	return i;
}

/**
 * This function is called whenever a process tries to do an Input/Output Control on our device file.
 * We get two extra parameters:
 * @param ioctl_num	The number of the ioctl called
 * @param ioctl_param	The parameter given to the ioctl function
 *
 * If the ioctl is write or read/write (meaning that the output is returned to the calling 
 * process), the ioctl call returns the output of this function.
 */
int device_ioctl(struct inode* inode, struct file* file, unsigned int ioctl_num, unsigned long ioctl_param)
{
	// Not sure if we can printk here. Will try after
	//printk(KERN_INFO "device_ioctl(%p, %p, %d, %d)\n", inode, file, ioctl_num, ioctl_param);
	
	int i;
	char* temp;
	char ch;

	// Switch according to the ioctl called.
	switch(ioctl_num)
	{
	case IOCTL_SET_MSG:

		temp = (char*)ioctl_param;

		// Find the length of the message.
		get_user(ch, temp);
		for(i = 0; ch && i < BUF_LEN; i++, temp++)
		{
			get_user(ch, temp);
		}

		device_write(file, (char*)ioctl_param, i, 0);
		break;

	case IOCTL_GET_MSG:
		// Give the current message to the calling process.
		// The parameter we got is a pointer, we need to fill it.
		i = device_read(file, (char*)ioctl_param, 99, 0);

		put_user('\0', (char*)ioctl_param + i);
		break;
	
	case IOCTL_NTH_BYTE:
		// This ioctl is both input (ioctl_param) and output (the return value
		// of this function).
		return Message[ioctl_param];
		break;
	}

	return SUCCESS;
}


static struct file_operations fops = {
	.read = device_read,
	.write = device_write,
	.ioctl = device_ioctl,
	.open = device_open,
	.release = device_release // close
};

// Function called when the module is loaded.
int init_module(void)
{

	int ret_val;

	// Register the character module statically this time..
	// This is done by giving this function the desired Major.
	ret_val = register_chrdev(MAJOR_NUM, DEVICE_NAME, &fops);
	
	if (ret_val < 0)
	{
		printk(KERN_ALERT "Registering char device failed with %d\n",ret_val);
		return ret_val;
	}

	// All good, let's inform the reader of the logs.
	printk(KERN_INFO "I was assigned major number: %d. To talk to\n", MAJOR_NUM);
	printk(KERN_INFO "the driver, create a dev fiile with\n");
	printk(KERN_INFO "mknod /dev/%s c %d 0.\n", DEVICE_NAME, MAJOR_NUM);
	printk(KERN_INFO "The device file name is important because the\n");
	printk(KERN_INFO "ioctl program assumes that's the device file.\n");
	printk(KERN_INFO "you'll use.\n");
	printk(KERN_INFO "Remove the device file and module when done.\n");

	return SUCCESS;
}

// This function gets called when the module is unloaded.
void cleanup_module()
{
	// Unregister the device.
	// this function returns void!
	unregister_chrdev(MAJOR_NUM, DEVICE_NAME);
}
