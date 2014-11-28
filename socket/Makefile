
KERNELDIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)
obj-m := thunderlink.o



build: user_thunder.o
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

%.o: %.c
	gcc -g $*.c -o $*.o

.PHONY: %clean
clean:
	@rm -f modules.order; \
	rm -f Module.symvers; \
	rm -f thunderlink.ko; \
	rm -f thunderlink.mod.c; \
	rm -f thunderlink.mod.o; \
	rm -f thunderlink.o; \
	rm -f user_thunder.o; \
	rm -f .thunderlink.ko.cmd; \
	rm -f .thunderlink.mod.o.cmd; \
	rm -f .thunderlink.o.cmd; \
	rm -rf .tmp_versions;




