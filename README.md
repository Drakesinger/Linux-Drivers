# Linux-Drivers

A repository containing Linux driver development experiments.

## Requirements

- A working Linux distribution.
- VIM

## Documentation

Used resources:
- [Linux Driver Programming](http://www.tldp.org/LDP/lkmpg/2.6/html/index.html)
- [Linux Device Drivers 2nd Edition](http://www.xml.com/ldd/chapter/book/index.html)
- [The Linux Information Project](http://www.linfo.org/index.html)
- [Kernel Newbies](http://kernelnewbies.org/Documents)
- vimtutor
- [VIM Docs](http://www.vim.org/docs.php)

Kernel documentation itself can be downloaded from Linux Kernel (take the version you want to make your driver for) source code at:
```
www.kernel.org
```
You need to extract the sources:
```bash
tar -xf linux-4.2.5.tar.xz
cd linux-4.2.5
```
In order to help you, you should make the documentation:
```bash
$ make mandocs        # For man files.
$ make installmandocs # To install the manual files.
```

## Theory

# TODO
--------------------
## Implementation

## Basics

### Headers

> #### A note about headers.
>
> You may want to look at the sources of your installed kernel (and not only the downloaded one), in order to find it:
> ```bash
cd /usr/src/linux-headers[version]-common/include/linux/
```
>
> If these headers are not available:
>```bash
apt-cache search linux-headers-$(uname -r)
apt-get install linux-headers-$(uname -r)
```
>

Required headers for all modules:

```c
#include <linux/module.h>
#include <linux/kernel.h>
```

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

Since we want to implement a char driver that takes in a string supplied by the user and returns it in inverse we will need to implement the open, release, write and read functions.

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
