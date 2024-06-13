obj-m += src/hxen.o src/xensocket.c
KERNEL_SRC_DIR="/usr/src/kernels/`uname -r`"

all:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean