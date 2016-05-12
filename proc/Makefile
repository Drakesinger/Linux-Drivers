#obj-m += hello-1.o
#obj-m += hello-2.o
#obj-m += hello-3.o
#obj-m += hello-4.o
#obj-m += hello-5.o

# Multi file object.
#obj-m += startstop.o
#startstop-objs := start.o stop.o

#obj-m += chardev.o
#obj-m += procfs1.o
#obj-m += procfs2.o

#IOCTLs
obj-m := char_comp.o
char_comp-y := src/char_dev.o
char_comp-y += src/io/ioctl.o
# Flags for include directories
ccflags-y := -I$(src)/include

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

spec:
	gcc -Wall -o hello hello.c

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm hello
