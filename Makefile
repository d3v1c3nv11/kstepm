obj-m += a20stepm.o

all:
	make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- -j8 -C /home/chris/a20/a20-olimex/linux-sunxi/out/lib/modules/3.4.103-00033-g9a1cd03-dirty/build M=$(PWD) modules
clean:
	make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- -j8 -C /home/chris/a20/a20-olimex/linux-sunxi/out/lib/modules/3.4.103-00033-g9a1cd03-dirty/build M=$(PWD) clean

