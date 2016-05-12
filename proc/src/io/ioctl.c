/*
 * ioctl.c - the process to use ioctls to control the kernel module
 *
 * Until now we could have used cat for input and output.
 * But now we need to do ioctls, which require writing our own
 * process.
 */

/*
 * Device specifics, such as:
 * - ioctl numbers
 * - Major device file.
 */
#include "include/char_dev.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>	// open.
#include <unistd.h>	// exit.
#include <sys/ioctl.h>	// ioctl.

/*
 * Functions for the IOCTL calls.
 */

ioctl_set_msg(int file_desc, char* message)
{
	int ret_val;

	ret_val = ioctl(file_desc, IOCTL_SET_MSG, message);

	if(ret_val < 0)
	{
		printf("ioctl_set_msg failed:%d\n", ret_val);
		exit(-1);
	}
}


ioctl_get_msg(int file_desc)
{
	int ret_val;
	char message[100];

	/*
	 * WARNING
	 *
	 * This is dangerous because we don't tell the kernel
	 * how far it's allowed to write, so it might overflow
	 * the buffer.
	 * In a real production program, we would have used 
	 * 2 IOCTLs - one to tell the kernel the buffer length
	 * and another to give it the buffer to fill.
	 */
	ret_val = ioctl(file_desc, IOCTL_GET_MSG, message);

	if(ret_val < 0)
	{
		printf("ioctl_get_msg failed:%d\n", ret_val);
		exit(-1);
	}

	printf("get_msg message:%s\n", message);
}


ioctl_nth_byte(int file_desc)
{
	int i = 0;
	char c;

	printf("get_nth_byte message:");

	do
	{
		c = ioctl(file_desc, IOCTL_NTH_BYTE, i++);

		if(c < 0)
		{
			printf("ioctl_get_nth_byte failed at the %d'th byte:\n", i);
			exit(-1);
		}

		// Print the character.
		putchar(c);
	}while(c != 0);

	putchar('\n');
}


// Main - Call the IOCTL functions.
main()
{
	int file_desc, ret_val;
	char* msg = "Message passed by ioctl\n";

	// Check man ioctl for some warning about open().
	file_desc = open(DEVICE_FILE_NAME, 0);
	if(file_desc < 0)
	{
		printf("Can't open device file: %s\n", DEVICE_FILE_NAME);
		exit(-1);
	}

	ioctl_nth_byte(file_desc);
	ioctl_get_msg(file_desc);
	ioctl_set_msg(file_desc, msg);

	close(file_desc);
}
