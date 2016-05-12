/*
 * procfs1.c - create a "file" in /proc
 */

#include <linux/module.h> 	/* Specifically, a module.	 */
#include <linux/kernel.h> 	/* We're doing kernel work.	 */
#include <linux/proc_fs.h> 	/* Necessary because we use the proc fs. */

#define procfs_name "helloworld"

/**
 * This structure holds information about the /proc file.
 */
struct proc_dir_entry* Our_Proc_File;

static const struct file_operations proc_file_fops = {
	.owner = THIS_MODULE
	//.open = open_callback,
	//.read = read_callback,
};

/*
 * Put the data into the proc fs file.
 * 
 * Arguments
 * =========
 * 1.	The buffer where the data is to be inserted, if you decide to use it.
 * 2.	A pointer to a pointer of characters.
 * 	This is useful if you don't want to use the buffer allocated
 * 	by the kernel.
 * 3.	The current position in the file.
 * 4.	The size of the buffer in the first argument.
 * 5.	Write a "1" here to indicate an EoF.
 * 6.	A pointer to data
 * 	Useful in case of common read for multiple /proc/... entries
 *
 * Usage and Return Value
 * ======================
 * A return value of zero means you have no further information at this time (EoF).
 * A negative value is an error condition.
 *
 * For more Information
 * ====================
 * The way I(Peter Jay Salzman) discovered what to do with this function
 * wasn't by reading the documentation, but by reading code which used it.
 * I just looked to see what uses the get_info field of proc_dir 
 * entry struct (I used a combination of find and grep, fyi), and I saw 
 * that it is used in <kernel source directory>/fs/proc/array.c.
 *
 * If something is unknown about the kernel, this is usually the way to go.
 * In Linux we have the great advantage of having the kernel source code
 * for free => USE IT.
 */
int procfile_read(char* buffer, 
		  char** buffer_location, 
		  off_t offset, 
		  int buffer_length, int* eof, void* data)
{

	int ret;

	printk(KERN_INFO "procfile_read (/proc/%s) called\n", procfs_name);

	/*
	 * We give all of our information in one go,
	 * so if the user asks us if we have more info
	 * the answer should always be no.
	 *
	 * This is important because the standard read
	 * function from the library would continue to issue
	 * the read system call *UNTIL THE KERNEL REPLIES
	 * THAT IT HAS NO MORE INFORMATION, OR UNTIL IT'S
	 * BUFFER IS FILLED.*
	 */
	if (offset > 0)
	{
		// We have finished reading, return 0
		ret = 0;
	}
	else
	{
		// Fill the buffer, return the buffer size
		ret = sprintf(buffer, "HelloWorld!\n");
	}

	return ret;
}

int init_module()
{
	Our_Proc_File = proc_create("proc_file_name", 0, NULL, &proc_file_fops);
	//Our_Proc_File = create_proc_entry(procfs_name, 0644, NULL);

	if (Our_Proc_File == NULL)
	{
		//remove_proc_entry(procfs_name, &proc_root);
		remove_proc_entry(procfs_name, NULL);
		printk(KERN_ALERT "Error: Could not initialize /proc/%s\n", procfs_name);
		return -ENOMEM;
	}

	// More stuff to do
	Our_Proc_File->read_proc = procfile_read;
	//Our_Proc_File->owner	 = THIS_MODULE; // This does not exist anymore.
	Our_Proc_File->mode	 = S_IFREG | S_IRUGO; // No idea what there are. Yet.
	Our_Proc_File->uid	 = 0;
	Our_Proc_File->gid	 = 0;
	Our_Proc_File->size	 = 37;
	
	printk(KERN_INFO "/proc/%s created\n", procfs_name);
	return 0;
}

void cleanup_module()
{
	//remove_proc_entry(procfs_name, &proc_root);
	remove_proc_entry(procfs_name, NULL); 
	printk(KERN_INFO "/proc/%s removed\n", procfs_name);
}
