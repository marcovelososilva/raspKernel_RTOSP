MYPROC=rtosp
obj-m += $(MYPROC).o

export KROOT=/lib/modules/$(shell uname -r)/build
#export KROOT=/lib/modules/$(uname)3.2.0-23-generic/build

allofit:  modules

modules: clean
	@$(MAKE) -C $(KROOT) M=$(PWD) -j4 modules

modules_install:
	@$(MAKE) -C $(KROOT) M=$(PWD) -j4 modules_install

kernel_clean:
	@$(MAKE) -C $(KROOT) M=$(PWD) clean

install:modules_install

clean: kernel_clean
	rm -rf   Module.symvers modules.order

insert: modules
	sudo dmesg -c
	sudo insmod $(MYPROC).ko

remove: clean
	sudo rmmod $(MYPROC)

	