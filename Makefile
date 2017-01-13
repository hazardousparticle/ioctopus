obj-m += ioctopus.o

KVERSION = $(shell uname -r)

all:
	make -C /lib/modules/$(KVERSION)/build M=$(PWD) modules
	strip --strip-debug *.ko

clean:
	make -C /lib/modules/$(KVERSION)/build M=$(PWD) clean

solution:
	gcc solution.c -o solution
