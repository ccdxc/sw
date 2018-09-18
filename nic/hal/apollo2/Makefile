default: all

ifndef TOPDIR
export TOPDIR := $(realpath ${CURDIR}/../../)
endif

%/.dir:
	$(Q)mkdir -p $(@D) && touch $@

include $(TOPDIR)/make/Makefile.common

BZL_BLD_OPT := $(BZL_BLD_OPT) --define p4pipeline=apollo
BZL_BLD_CMD := bazel $(BZL_OPT) build $(BZL_BLD_OPT)

plugins:
	$(BZL_BLD_CMD) --build_tag_filters=-gtests //nic/hal/plugins/...

####################################################
#################### Oracle ########################
####################################################

apollo:
	$(MAKE) capmodel
	$(MAKE) -j4 -C $(TOPDIR)/p4 apollo_p4
	$(MAKE) -C $(TOPDIR)/asm/apollo
	$(MAKE) gen_proto
	$(MAKE) -j4 build_proto
	$(MAKE) sdk
	$(MAKE) linkmgr
	$(MAKE) third-party
	$(MAKE) hal
	$(BZL_BLD_CMD) //nic/hal/test/gtests:apollo_test
	$(BZL_BLD_CMD) //nic/utils/host_mem:all

all: apollo

clean: ;
