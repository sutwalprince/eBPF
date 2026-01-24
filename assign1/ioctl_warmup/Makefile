obj-m += simple_ioctl.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

load:
	sudo insmod simple_ioctl.ko

unload:
	sudo rmmod simple_ioctl

reload: unload load

logs:
	sudo dmesg | tail -20