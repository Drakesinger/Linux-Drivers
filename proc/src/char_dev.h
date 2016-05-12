/*
 * char_dev.h - The header file with the IOCTL definitions.
 *
 * The declarations here have to be in a header file, because they
 * need to be known both by the kernel module in char_dev.c and the process
 * calling ioctl (ioctl.c)
 */

#ifndef CHAR_DEV_H
#define CHAR_DEV_H

#include <linux/ioctl.h>

// The major device number.
// We can't rely on dynamic registration any more beauce
// the IOCTLs need to know it.
#define MAJOR_NUM 100

// Set the message of the device driver.
#define IOCTL_SET_MSG _IOR(MAJOR_NUM, 0, char*)
/*
 * _IOR means we're creating a ioctl command number
 * for passing information from a user process to the kernel
 * module.
 *
 * The 1st argument: MAJOR_NUM, is the major device number we're using.
 *
 * The 2nd argument: 0, is the number of the command (there could be
 * several with different meanings).
 *
 * The 3rd argument: char*, is the type we want to get from the user process
 * to the kernel.
 */

// Get the message of the device driver.
#define IOCTL_GET_MSG _IOR(MAJOR_NUM, 1, char*)
/*
 * This IOCTL is used for output, to get the message of the device driver.
 * However, we still need the buffer to place the message in to be input,
 * as it is allocated by the process.
 */

// Get the n'th byte of the message.
#define IOCTL_GET_NTH_BYTE _IOR(MAJOR_NUM, 2, int)
/*
 * The IOCTL is used for both input and output.
 * It received from the user a number, n, and returns
 * Message[n].
 */

// Name of the device file.
#define DEVICE_FILE_NAME "char_dev"

#endif
