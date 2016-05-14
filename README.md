
# Linux-Drivers

A repository containing Linux driver development experiments.

## Table of Contents

<!-- TOC depthFrom:2 depthTo:6 withLinks:1 updateOnSave:1 orderedList:0 -->

- [Table of Contents](#table-of-contents)
- [Requirements](#requirements)
- [Documentation](#documentation)
- [Theory](#theory)
	- [Memory](#memory)
	- [Supervisor vs User Mode](#supervisor-vs-user-mode)
- [Implementation](#implementation)
	- [Headers](#headers)
	- [Initialization and Cleanup functions](#initialization-and-cleanup-functions)
	- [File and File Operations](#file-and-file-operations)
		- [File Struct](#file-struct)
		- [File Operations](#file-operations)
	- [Registering your device driver](#registering-your-device-driver)
	- [Driver Functionality](#driver-functionality)
		- [Device Open](#device-open)
		- [Device release](#device-release)
		- [Device Read](#device-read)
		- [Device Write](#device-write)
		- [Testing](#testing)
			- [What's left](#whats-left)

<!-- /TOC -->

## Requirements

- A working Linux distribution.
- VIM

## Documentation

Used resources:
- [Linux Driver Programming](http://www.tldp.org/LDP/lkmpg/2.6/html/index.html)

  This one has been used the most.
- [Linux Device Drivers 2nd Edition](http://www.xml.com/ldd/chapter/book/index.html)

  But this one is the best book that you should read.

- [Free Electrons Linux Kernel Cross Reference](http://lxr.free-electrons.com/)

  Allows searching for functions and finds their definitions and declarations.
  Not one of the official cross reference projects but the 1st one that is easy to use directly.
- [Linux Kernel Hacking](https://www.kernel.org/doc/htmldocs/kernel-hacking/index.html)
- [The Linux Information Project](http://www.linfo.org/index.html)
- [Kernel Newbies](http://kernelnewbies.org/Documents)
- vimtutor
- [VIM Docs](http://www.vim.org/docs.php)

Kernel documentation itself can be downloaded from the Linux Kernel source code (take the version you want to make your driver for) located at:
```
www.kernel.org
```
You need to extract the sources:
```bash
tar -xf linux-[version].tar.xz
cd linux-[version]
```
In order to help you, you should make the documentation:
```bash
$ make mandocs        # For man files.
$ make installmandocs # To install the manual files.
```

## Theory

### Memory
Here is a basic representation of how the kernel and memory works.
The kernel represents the running kernel software that is in Memory (RAM).

```
+---+ +-----------------------------------------------Memory------------------------------------------------+
|   | |                                                 |                                                   |
|   | |               Kernel Space                      |                     User Space                    |
|   | |                                                 |                                                   |
|   | |                                                 |                                                   |
|   | |                                                 |   bad practice  +-----------------+               |
|   | |                                                 |    +------------+  User Processes |               |
|   | |           +-----------------+                   |    |            +------+---^------+               |
|   | |           |                 <------------+      |    |                   |   |                      |
| H | |     +-----+     Kernel      |            |      |    |                   |   |                      |
| a | |     |     |                 |         +--+------+----v---+               |   |                      |
| r | |     |     +-----------------+         |   System Calls   |               |   |                      |
| d | |     |                                 +--+------+---^-+--+               |   |                      |
| w | |     |                                    |      |   | |                  |   |                      |
| a | |     |         +---------+                |      |   | |              +---v---+---+                  |
| r | |     |         |Scheduler+----------------+      |   | +-------------->  Library  |                  |
| e | |     |         +---------+                       |   +----------------+ Functions |                  |
|   | |     |                                           |                    |  (glibc)  |                  |
|   | |     +-------------+                             |                    +-----------+                  |
|   | |                   |                             |                                                   |
|   | |             +-----v-------+                     |                                                   |
|   <---------------+   Drivers   |                     |                                                   |
|   | |             +-------------+                     |                                                   |
|   | |                                                 |                                                   |
+---+ +-------------------------------------------------+---------------------------------------------------+
```

In order for a running program instance (process) to communicate with the hardware, one or more system calls will be executed. These system calls are called by the library implementation [`glibc`](https://www.gnu.org/software/libc/) in the order defined by the Kernel's Scheduler. One can call them directly but it is considered bad practice (the `glibc` implementation is considered to be optimized for these calls).

The system calls are run by the Kernel and their result is then provided to the user process.

Want to see what system calls are used to run your `printf("Hello World!");`?

Do the following (you can write this as is in the console):
```bash
$ cat << EOF > hello.c
#include <stdio.h>
int main(void)
{  
  printf("hello");
  return 0;
}
EOF

gcc -Wall -o hello hello.c
strace ./hello
```
See the `write(...)` at the end? That's one System Call.

> More information on the GNU C Library can be found at [GNU Lib C](https://www.gnu.org/software/libc/).

> For information on Windows C Library go to [MSDN - C Run-Time Library Reference](https://msdn.microsoft.com/en-us/library/59ey50w6.aspx).

### Supervisor vs User Mode
It should be of note that User Space applications and processes run in User Mode, where they can change the memory in User Space.

The Kernel runs in Supervisor mode and the system calls made run in this mode on behalf of the user. Everything goes in Supervisor mode. The running Kernel software has access to the entire memory and can delete whatever it wants (although normally this is not the case). It can also decide to allow a process for example to execute in DMA ([Direct Memory Access](https://en.wikipedia.org/wiki/Direct_memory_access)) mode.
So when writing a driver, care must be taken so that you don't mess up your system.

## Implementation

### Headers

> #### A note about headers.
>
> You may want to look at the sources of your installed kernel (and not only the downloaded one).<br> In order to find them:
> ```bash
$ cd /usr/src/linux-headers[version]-common/include/linux/
```
>
> If these headers are not available:
>```bash
$ apt-cache search linux-headers-$(uname -r)
$ apt-get install linux-headers-$(uname -r)
```
>

Required headers for all modules:

```c
#include <linux/module.h>
#include <linux/kernel.h>
```
The module header is required by all modules.
The kernel header contains the kernel functions that your module can use.

### Initialization and Cleanup functions

Both of these functions are mandatory and the driver will not work without them.
Their names and signatures cannot be changed.
```c
int init_module(void)
{
  // Module registration is done here.

  // A non 0 return means an error arrived and the module will not be added.
  return 0;
}

void cleanup_module(void)
{
  // Module un-registration is done here.
}
```

There is a macro that allows you to define your **initialization** and **cleanup** function with another name.

In order to use the macro we need to include another header:

```c
#include <linux/init.h> // For the macros.

static int __init initFunction()
{
  // Instructions here.
  return 0;
}

static void __extern exitFunction()
{
  // Instructions here.
}

// Macros.
module_init(initFunction);
module_exit(exitFunction);
```

### File and File Operations
#### File Struct

Each device driver is represented by a `file` structure.
This kernel level structure is defined in:
```c
#include <linux/fs.h>
```

Do note that this is not the same as `FILE`, which is defined by the `libc` library.

#### File Operations

Now is the time to define which operations we want to implement for our driver.
These operations are located in another structure named `file_operations`.
This structure is also defined in the `fs.h` header.

Here are the possible functions that we can implement:
```
/* TODO */
```

Since we want to implement a char driver that takes in a string supplied by the user and returns it's reverse we will need to implement the `open`, `release`, `write` and `read` functions.

```c
/*
 * Our defined operations that are implemented.
 * All others are NULL.
 */
static struct file_operations fops = {
	.read = device_read,
	.write = device_write,
	.open = device_open,
	.release = device_release
};
```

### Registering your device driver

If you look at some device drivers:
```bash
$ ls -l /dev/
```
You'll get an output such as:
```bash
crw------T  1 root root     10, 235 May 11 19:59 autofs
drwxr-xr-x  2 root root         300 May 11 19:59 block
drwxr-xr-x  2 root root          80 May 11 19:59 bsg
crw------T  1 root root     10, 234 May 11 19:59 btrfs-control
drwxr-xr-x  3 root root          60 May 11 19:59 bus
lrwxrwxrwx  1 root root           3 May 11 19:59 cdrom -> sr0
```

The column with numbers in the middle, before the date and after the group field, you have information about which driver handles this device.

1. Major number
2. Minor number

The Kernel itself only cares about the Major number: it specifies the
 driver that is this device's handler.
The Minor number is used by the driver itself to handle different types of devices.

We can see here that `autofs` and `btrfs-control` is handled by the same driver. The two different file systems are handled differently (probably) by the driver by identifying each device with a different minor number.

Too see which Major numbers are available and which number represents what, read the following file:

`[linux kernel source path]/Documentation/devices.txt`.

In order to register our device driver we need to use the following function:
```c
int  register_chrdev(int MAJOR_NUMBER, const char* DEVICE_NAME, struct file_operations* fops);
```

This will return a positive value containing the registered Major number if the registration was accepted by the kernel and a negative one otherwise.
If the MAJOR_NUMBER provided is 0, the kernel will dynamically assign a Major Number to the driver.

Once the registration is done, we need to make a link to the device driver with the following command:
```bash
$ sudo mknod /dev/[DEVICE_NAME] c [Major Number provided] 0
```

This will make a link to our device with the dynamic major and the minor 0.

During cleanup, we need to unregister our driver with:
```c
void unregister_chrdev(int MAJOR_NUMBER, const char* DEVICE_NAME);
```

This function always returns.

### Driver Functionality

This snipets below are just that: snipets. In order to see the full code, check out the file `./char/char_dev.c`.

#### Device Open

`Device_Open` is a static integer belonging to this driver that increments each time the device file is opened and decrements each time it is released.

```c
// Called when a process tries to open the device file like: "cat /dev/chardev".
static int device_open(struct inode* inode, struct file* filp)
{
	static int counter = 0;

  // We only want one process to have this device driver open at a time.
  // All other requests will receive the -EBUSY status.
	if (Device_Open)
	{
		return -EBUSY;
	}

	Device_Open++;

	// Inform the user how many times this device driver file has been opened.
  printk(KERN_INFO "device_open() has been called %d times.\n", counter++);

  // Make our pointers point to the correct place.
	msg_read_Ptr = msg_read;
	msg_write_Ptr = msg_write;

  // Check if this module has been removed or not.
  // If it fails, the module is or has been removed
  // and it is better to assume it isn't there any more.
  try_module_get(THIS_MODULE);

	return 0;
}
```

#### Device release
```c
// Called when a process closes the device file.
static int device_release(struct inode* inode, struct file* file)
{
	printk(KERN_INFO "device_release()\n");
	Device_Open--; // We're now ready for our next caller.

	// Decrement the usage count, or else once you opened the file,
	// you'll never get rid of the module.
	module_put(THIS_MODULE);
	return 0;
}

```

`module_put` acts as a semaphore and decrements the counter for the module.

#### Device Read

```c
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
		 * so "*" assignment won't work.
     * We have to use put_user
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
```

Here we have an important function:

```c
put_user(const char* source, char __user* destination).
```

This is how you pass data from user space to kernel space.
The process that calls read on this device driver is in user space and requires data from the kernel space. So we put into user space the data from source to destination.
The `__user` has been omitted but it can be inserted to be able to differentiate the two memory spaces.

There is a defect in this function that has not been fixed.
Once the buffer has been read, it should be cleared. It makes no point to be able to read again and again the same data. In our case it's ok, but it should be avoided.

#### Device Write

In this function, we will take the buffered data sent by the user to our driver, we will reverse it, and store it in msg_read (which will be returned to the user when (s)he reads from the device).

```c
// Called when a process writes to dev file: echo "hi" > /dev/chardev
static ssize_t device_write(struct file* filp, const char __user*  buff, size_t len, loff_t* off)
{
	int i;
  int j;
	int bytes_written = 0;

  // Some debugging.
	printk(KERN_INFO "device_write()\n");

	// Get message from user data segment.
	for(i = 0; i < len && i < BUF_LEN; i++)
	{
		// Fill the buffer from the user data segment.
		// copy the values from the pointer buff to msg_write.
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

	msg_read_Ptr = msg_read;
	msg_write_Ptr = msg_write;

	return bytes_written;
}
```

We just saw the function used to transfer data from user space to kernel space:

```c
get_user(char __user* destination, const char* source).
```

The data from user space buffer `source` is copied to the buffer in kernel space.

#### Testing

We can now run the Makefile and insert our module.
Once the module is inserted with:
```bash
$ sudo insmod char_dev.ko
```

We can check the kernel logs to see how to make the link to our device driver we just created:
```bash
$ sudo tail -n 10 /var/log/messages
```

In order to have write access, we need to change the permissions on the node.
```bash
$ sudo chmod 666 /dev/chardev
```

Once this is done, try the following:
```bash
$ cat /dev/chardev
$ echo "Heya there." > /dev/chardev
$ cat /dev/chardev
```

##### What's left

1. Make a c program to illustrate reading and writing to this device driver.
> ```c
int main(int argc, char* argv[])
{
  int i = 0;
  int filep = open("/dev/chardev", 0_RDWR);
  write(filep, "Stuff", strlen("Stuff"));
  while(read(filep, &destination_str[i++], 1));
  printf("Reverse is: %s\n", destination_str);
}
```

2. IOCTL
  Sadly this part failed due to my bad understanding of the `kbuild` system.
  The information from Linux Driver Programming is not enough to be able to do this part and didn't have time yet to study the Linux Device Drivers book in full.
3. System Calls
  Same as before, this part however looks very interesting and will surely be addressed soon.
