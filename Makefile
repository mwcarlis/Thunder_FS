
#WARN 	:= -W -Wall -Wstrict-prototypes -Wmissing-prototypes
#INCLUDE := -isystem /lib/modules/`uname -r`/build/include
#CFLAGS 	:= -O2 -DMODULE -D__KERNEL__${WARN} ${INCLUDE}
#CC 	:= gcc
KERNELDIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)
obj-m := thunderfs.o



build:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

%.o: %.c
	echo $(obj-m)
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

.PHONY: %Clean
clean:
	@rm -f modules.order; \
	rm -f Module.symvers; \
	rm -f thunderfs.ko; \
	rm -f thunderfs.mod.c; \
	rm -f thunderfs.mod.o; \
	rm -f thunderfs.o;



.PHONY: %Clean
%Clean:
	rm -f modules.order; \
	rm -f Modules.symvers; \
	rm -f $*.ko; \
	rm -f $*.mod.c; \
	rm -f $*.mod.o; \
	rm -f $*.o;

%Test:
	@echo $@
	@echo $*


