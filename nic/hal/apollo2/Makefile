default: all

ifndef TOPDIR
export TOPDIR := $(realpath ${CURDIR}/../../)
endif

%/.dir:
	$(Q)mkdir -p $(@D) && touch $@

include $(TOPDIR)/make/Makefile.common

BZL_BLD_OPT := $(BZL_BLD_OPT) --define p4pipeline=apollo2
BZL_BLD_CMD := bazel $(BZL_OPT) build $(BZL_BLD_OPT)

####################################################
#################### Oracle ########################
####################################################

apollo2: capsim-master
	$(MAKE) capmodel
	$(MAKE) -j4 -C $(TOPDIR)/p4 apollo2_p4
	$(MAKE) -C $(TOPDIR)/asm/apollo2
	$(MAKE) gen_proto
	$(MAKE) -j4 build_proto
	$(MAKE) sdk
	$(MAKE) linkmgr
	$(BZL_BLD_CMD) //nic/hal:hal
	$(BZL_BLD_CMD) //nic/hal/test/gtests:apollo2_test
	$(BZL_BLD_CMD) //nic/utils/host_mem:all

all: apollo2

clean: ;
