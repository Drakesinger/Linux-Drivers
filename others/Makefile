#obj-m += hello-1.o
#obj-m += hello-2.o
#obj-m += hello-3.o
#obj-m += hello-4.o
#obj-m += hello-5.o

# Multi file object.
#obj-m += startstop.o
#startstop-objs := start.o stop.o

obj-m += char_dev.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

spec:
	gcc -Wall -o hello hello.c

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

cleans:
	rm hello
