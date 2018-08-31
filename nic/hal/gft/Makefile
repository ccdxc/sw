default: all

ifndef TOPDIR
export TOPDIR := $(realpath ${CURDIR}/../../)
endif

%/.dir:
	$(Q)mkdir -p $(@D) && touch $@

include $(TOPDIR)/make/Makefile.common

BZL_BLD_OPT := $(BZL_BLD_OPT) --copt=-DGFT --define p4pipeline=gft

BAZEL_BUILD := bazel $(BZL_OPT) build $(BZL_BLD_OPT)

#################################################
################## GFT ##########################
#################################################

swig-cli-gft-clean:
	make -C $(TOPDIR)/debug_cli clean PGM=gft
	make -C $(TOPDIR)/debug_cli clean PGM=common_txdma_actions
	make -C $(TOPDIR)/debug_cli clean PGM=common_rxdma_actions
	make -C $(TOPDIR)/debug_cli clean PGM=cli

swig-cli-gft: swig-cli-gft-clean
	PATH=$(TOOLCHAIN_PATH)/bin:$$PATH make -C $(TOPDIR)/debug_cli PGM=gft
	PATH=$(TOOLCHAIN_PATH)/bin:$$PATH make -C $(TOPDIR)/debug_cli PGM=common_txdma_actions
	PATH=$(TOOLCHAIN_PATH)/bin:$$PATH make -C $(TOPDIR)/debug_cli PGM=common_rxdma_actions
	PATH=$(TOOLCHAIN_PATH)/bin:$$PATH make -C $(TOPDIR)/debug_cli PGM=cli

plugins:
	$(BAZEL_BUILD) --build_tag_filters=-gtests //nic/hal/plugins/...

gft: platform
	$(MAKE) capmodel
	$(MAKE) -j4 -C $(TOPDIR)/p4 msft_gft
	$(MAKE) -C $(TOPDIR)/asm msft_gft AS_DEFINES="-DGFT -DCAPRI_IGNORE_TIMESTAMP"
	$(MAKE) gen_proto
	$(MAKE) -j4 build_proto
	$(MAKE) csrlite
	$(MAKE) sdk
	$(MAKE) linkmgr
	$(MAKE) third-party
	$(MAKE) hal
	$(BAZEL_BUILD) //nic/hal/pd/gft:all
	$(BAZEL_BUILD) //nic/hal/pd/pd_stub
	$(MAKE) gtests_gft
	$(MAKE) swig-cli-gft
	$(BAZEL_BUILD) //nic/hal/test/gtests:gft_test
	$(BAZEL_BUILD) //nic/utils/host_mem:all
	$(MAKE) post-make

gtests_gft:
	$(BAZEL_BUILD) //nic/hal/test/gtests:lif_gft_test
	$(BAZEL_BUILD) //nic/hal/test/gtests:uplinkif_gft_test
	$(BAZEL_BUILD) //nic/hal/test/gtests:enicif_gft_test
	$(BAZEL_BUILD) //nic/hal/test/gtests:endpoint_gft_test
	$(BAZEL_BUILD) //nic/hal/test/gtests:emp_gft_test
	$(BAZEL_BUILD) //nic/hal/test/gtests:efe_gft_test
	 @# Once PD is properly cleaned up gft_test should be compiled with the below cmd. Remove above cmds
	#bazel build --build_tag_filters=gft //nic/hal/test/gtests:all

run-gft-gtests: $(GEN_TEST_RESULTS_DIR)/.dir
	$(CMD_OPTS) ./nic/hal/test/gtests/lif_gft_test --gtest_output="xml:$(GEN_TEST_RESULTS_DIR)/lif_gft_test.xml"
	$(CMD_OPTS) ./nic/hal/test/gtests/uplinkif_gft_test --gtest_output="xml:$(GEN_TEST_RESULTS_DIR)/uplinkif_gft_test.xml"
	$(CMD_OPTS) ./nic/hal/test/gtests/enicif_gft_test --gtest_output="xml:$(GEN_TEST_RESULTS_DIR)/enicif_gft_test.xml"
	$(CMD_OPTS) ./nic/hal/test/gtests/endpoint_gft_test --gtest_output="xml:$(GEN_TEST_RESULTS_DIR)/endpoint_gft_test.xml"
	$(CMD_OPTS) ./nic/hal/test/gtests/emp_gft_test --gtest_output="xml:$(GEN_TEST_RESULTS_DIR)/emp_gft_test.xml"
	$(CMD_OPTS) ./nic/hal/test/gtests/efe_gft_test --gtest_output="xml:$(GEN_TEST_RESULTS_DIR)/efe_gft_test.xml"

all: gft

clean: ;
