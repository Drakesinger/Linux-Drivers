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

### Supervisor vs User Mode
It should be of note that User Space applications and processes run in User Mode, where they can change the memory in User Space.

The Kernel runs in Supervisor mode and the system calls made run in this mode on behalf of the user. Everything goes in Supervisor mode. The running Kernel software has access to the entire memory and can delete whatever it wants (although normally this is not the case). It can also decide to allow a process for example to execute in DMA ([Direct Memory Access](https://en.wikipedia.org/wiki/Direct_memory_access)) mode.
So when writing a driver, care must be taken so that you don't mess up your system.

## Implementation

## Basics

### Headers

> #### A note about headers.
>
> You may want to look at the sources of your installed kernel (and not only the downloaded one).<br> In order to find them:
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
