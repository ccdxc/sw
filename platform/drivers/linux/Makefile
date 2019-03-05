ifeq ($(ARCH),aarch64)
include ${MKINFRA}/config_${ARCH}.mk
endif

ifneq ($(KERNELRELEASE),)
obj-$(CONFIG_IONIC) += eth/ionic/
obj-$(CONFIG_IONIC_MNIC) += eth/ionic/
obj-$(CONFIG_MNET) += mnet/
obj-$(CONFIG_INFINIBAND_IONIC_RDMA) += rdma/drv/ionic/
else

TOPDIR=../../

default: all

IONIC = ionic
IONIC_ETH_SRC = $(CURDIR)/eth/$(IONIC)
IONIC_RDMA_SRC = $(CURDIR)/rdma/drv/$(IONIC)

# Override with `make KSRC=/path/to/your/sources`
# or `export KSRC=/path/to/your/sources`

ifeq ($(ARCH),aarch64)
KSRC ?= $(AARCH64_LINUX_HEADERS)
KOPT += CONFIG_IONIC_MNIC=m
KOPT += CONFIG_MNET=m
KOPT += CROSS_COMPILE=aarch64-linux-gnu-
export PATH := $(PATH):$(TOOLCHAIN_DIR)/bin
else
KSRC ?= /lib/modules/$(shell uname -r)/build
KOPT += CONFIG_IONIC=m
endif

#KOPT += V=1		# verbose build
#KOPT += W=1		# extra warnings
#KOPT += C=1		# static analysis
#KOPT += CHECK=sparse	# static analysis tool

KOPT += KCPPFLAGS="-DHAPS"

# Select the rdma driver, too, if present
ifneq ($(ARCH),aarch64)
ifneq ($(wildcard rdma/drv/ionic/),)
KOPT += CONFIG_INFINIBAND_IONIC_RDMA=m
else
# Kbuild doesn't know about target obj-_, so this doesn't break "clean".
KOPT += CONFIG_INFINIBAND_IONIC_RDMA=_
endif
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
