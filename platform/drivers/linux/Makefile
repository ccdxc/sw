ifneq ($(KERNELRELEASE),)
obj-$(CONFIG_IONIC) += eth/ionic/
obj-$(CONFIG_INFINIBAND_IONIC_RDMA) += rdma/drv/ionic/
else

TOPDIR=../../

default: all

-include $(TOPDIR)/make/Makefile.docker

IONIC = ionic
IONIC_ETH_SRC = $(CURDIR)/eth/$(IONIC)
IONIC_RDMA_SRC = $(CURDIR)/rdma/drv/$(IONIC)

# Override with `make KSRC=/path/to/your/sources`
# or `export KSRC=/path/to/your/sources`
KSRC ?= /lib/modules/$(shell uname -r)/build

#KOPT += V=1		# verbose build
#KOPT += W=1		# extra warnings
#KOPT += C=1		# static analysis
#KOPT += CHECK=sparse	# static analysis tool

KOPT += KCPPFLAGS="-DHAPS"
KOPT += CONFIG_IONIC=m

# Select the rdma driver, too, if present
ifneq ($(wildcard rdma/drv/ionic/),)
KOPT += CONFIG_INFINIBAND_IONIC_RDMA=m
else
# Kbuild doesn't know about target obj-_, so this doesn't break "clean".
KOPT += CONFIG_INFINIBAND_IONIC_RDMA=_
endif

all:
	$(MAKE) -C $(KSRC) $(KOPT) M=$(CURDIR)

clean:
	$(MAKE) -C $(KSRC) $(KOPT) M=$(CURDIR) clean

install modules_install:
	$(MAKE) -C $(KSRC) $(KOPT) M=$(CURDIR) modules_install

cscope:
	find $(IONIC_ETH_SRC) $(IONIC_RDMA_SRC) -name '*.[ch]' > cscope.files
	cscope -bkq

.PHONY: all clean install modules_install cscope

endif
