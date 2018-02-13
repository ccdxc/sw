.PHONY: p4 asm proto gen hal utils linkmgr sdk

default: all

TOPDIR = .
TOPDIR_ABS = ${CURDIR}
CUSTOM = hal

include $(TOPDIR)/make/Make.inc
include $(TOPDIR)/make/Makefile.inc
include $(TOPDIR)/make/Makefile.docker
include $(TOPDIR)/make/Makefile.tests
include $(TOPDIR)/make/Makefile.jobs

#################################################
############## VALGRIND #########################
#################################################

VAL_CMD = valgrind --xml=yes 
VAL_CMD_OPTS = $(CMD_OPTS) $(VAL_CMD)
VALGRIND_DIR= $(TOPDIR_ABS)/gen/valgrind
VAL_TR = $(VALGRIND_DIR)/test_results
VAL_INFO = $(VALGRIND_DIR)/info
run-gtests-valgrind: $(VAL_TR)/.dir $(VAL_INFO)/.dir
	# PI gtests
	$(VAL_CMD_OPTS) --xml-file=$(VAL_INFO)/uplinkif_val.xml ./nic/hal/test/gtests/uplinkif_test --gtest_output="xml:$(VAL_TR)/uplinkif_test.xml"
	$(VAL_CMD_OPTS) --xml-file=$(VAL_INFO)/uplinkpc_val.xml ./nic/hal/test/gtests/uplinkpc_test --gtest_output="xml:$(VAL_TR)/uplinkpc_test.xml" 
	$(VAL_CMD_OPTS) --xml-file=$(VAL_INFO)/vrf_val.xml ./nic/hal/test/gtests/vrf_test --gtest_output="xml:$(VAL_TR)/vrf_test.xml" 
	$(VAL_CMD_OPTS) --xml-file=$(VAL_INFO)/acl_val.xml ./nic/hal/test/gtests/acl_test --gtest_output="xml:$(VAL_TR)/acl_test.xml"
	$(VAL_CMD_OPTS) --xml-file=$(VAL_INFO)/bm_allocator_val.xml ./nic/hal/test/gtests/bm_allocator_test --gtest_output="xml:$(VAL_TR)/bm_allocator_test.xml"
	$(VAL_CMD_OPTS) --xml-file=$(VAL_INFO)/enicif_val.xml ./nic/hal/test/gtests/enicif_test --gtest_output="xml:$(VAL_TR)/enicif_test.xml"
	$(VAL_CMD_OPTS) --xml-file=$(VAL_INFO)/lif_manager_val.xml ./nic/hal/test/gtests/lif_manager_test --gtest_output="xml:$(VAL_TR)/lif_manager_test.xml" 
	$(VAL_CMD_OPTS) --xml-file=$(VAL_INFO)/lif_val.xml ./nic/hal/test/gtests/lif_test --gtest_output="xml:$(VAL_TR)/lif_test.xml"
	$(VAL_CMD_OPTS) --xml-file=$(VAL_INFO)/session_val.xml ./nic/hal/test/gtests/session_test --gtest_output="xml:$(VAL_TR)/session_test.xml"
	$(VAL_CMD_OPTS) --xml-file=$(VAL_INFO)/tlscb_val.xml ./nic/hal/test/gtests/tlscb_test --gtest_output="xml:$(VAL_TR)/tlscb_test.xml"
	$(VAL_CMD_OPTS) --xml-file=$(VAL_INFO)/internal_val.xml ./nic/hal/test/gtests/internal_test --gtest_output="xml:$(VAL_TR)/internal_test.xml"
	$(VAL_CMD_OPTS) --xml-file=$(VAL_INFO)/endpoint_val.xml ./nic/hal/test/gtests/endpoint_test --gtest_output="xml:$(VAL_TR)/endpoint_test.xml"
	$(VAL_CMD_OPTS) --xml-file=$(VAL_INFO)/l2seg_val.xml ./nic/hal/test/gtests/l2seg_test --gtest_output="xml:$(VAL_TR)/l2seg_test.xml"
	$(VAL_CMD_OPTS) --xml-file=$(VAL_INFO)/tunnelif_val.xml ./nic/hal/test/gtests/tunnelif_test --gtest_output="xml:$(VAL_TR)/tunnelif_test.xml"
	$(VAL_CMD_OPTS) --xml-file=$(VAL_INFO)/nwsec_val.xml ./nic/hal/test/gtests/nwsec_test --gtest_output="xml:$(VAL_TR)/nwsec_test.xml"
	$(VAL_CMD_OPTS) --xml-file=$(VAL_INFO)/scheduler_tx_test.xml ./nic/hal/test/gtests/scheduler_tx_test --gtest_output="xml:$(VAL_TR)/scheduler_tx_test.xml"
	$(VAL_CMD_OPTS) --xml-file=$(VAL_INFO)/qos_val.xml ./nic/hal/test/gtests/qos_test --gtest_output="xml:$(VAL_TR)/qos_test.xml"
	$(VAL_CMD_OPTS) --xml-file=$(VAL_INFO)/copp_val.xml ./nic/hal/test/gtests/copp_test --gtest_output="xml:$(VAL_TR)/copp_test.xml"

#################################################
################ HAL-RTL ########################
#################################################

BZL_NON_DOCK_BLD_OPT := $(BZL_BLD_OPT) --linkopt=-L/usr/local/lib
hal-rtl: p4 platform capsim-master sdk
	$(MAKE) AS_DEFINES="-DCAPRI_IGNORE_TIMESTAMP" asm
	$(MAKE) proto
	$(MAKE) gen
	bazel $(BZL_OPT) build $(BZL_NON_DOCK_BLD_OPT) $(SDK_OPTS) hal:hal
	bazel $(BZL_OPT) build $(BZL_NON_DOCK_BLD_OPT) $(SDK_OPTS) hal/pd/iris:libiris.so
	bazel $(BZL_OPT) build $(BZL_NON_DOCK_BLD_OPT) $(SDK_OPTS) hal/pd/pd_stub
	bazel $(BZL_OPT) build $(BZL_NON_DOCK_BLD_OPT) $(SDK_OPTS) hal/plugins/...
	bazel $(BZL_OPT) build $(BZL_NON_DOCK_BLD_OPT) $(SDK_OPTS) //dol/test/storage:storage_test
	$(MAKE) utils
	$(MAKE) capmodel

#################################################
################## GFT ##########################
#################################################

gft: platform capsim-master sdk
	$(MAKE) capmodel
	$(MAKE) -j4 -C p4 msft_gft
	$(MAKE) -C asm msft_gft AS_DEFINES="-DGFT"
	$(MAKE) -C proto
	$(MAKE) -j4 -C gen/proto/hal
	bazel build --copt=-DGFT --define p4pipeline=gft $(SDK_OPTS) hal:hal
	$(MAKE) pd_gft
	$(MAKE) pd_stub
	$(MAKE) gtests_gft
	bazel build --copt=-DGFT --define p4pipeline=gft $(SDK_OPTS) hal/test/gtests:gft_test
	bazel $(BZL_OPT) build $(BZL_BLD_OPT) $(SDK_OPTS) utils/host_mem:all
	$(MAKE) post-make

gtests_gft:
	bazel build --copt=-DGFT --define p4pipeline=gft $(SDK_OPTS) hal/test/gtests:lif_gft_test
	bazel build --copt=-DGFT --define p4pipeline=gft $(SDK_OPTS) hal/test/gtests:uplinkif_gft_test
	bazel build --copt=-DGFT --define p4pipeline=gft $(SDK_OPTS) hal/test/gtests:enicif_gft_test
	 @# Once PD is properly cleaned up gft_test should be compiled with the below cmd. Remove above cmds
	#bazel build $(SDK_OPTS) --build_tag_filters=gft hal/test/gtests:all

run-gft-gtests: $(GEN_TEST_RESULTS_DIR)/.dir
	$(CMD_OPTS) ./nic/hal/test/gtests/lif_gft_test --gtest_output="xml:$(GEN_TEST_RESULTS_DIR)/lif_gft_test.xml"
	$(CMD_OPTS) ./nic/hal/test/gtests/uplinkif_gft_test --gtest_output="xml:$(GEN_TEST_RESULTS_DIR)/uplinkif_gft_test.xml"

#################################################
#################### HAL ########################
#################################################

swig-cli-clean:
	make -C gen/iris/cli -f Makefile.click clean

swig-cli: swig-cli-clean
	PATH=$(TOOLCHAIN_PATH)/bin:$$PATH make -C gen/iris/cli -f Makefile.click

package-clean:
	cd ../ && rm -rf fake_root_target && rm -f hal.tgz && rm -f hal.tar

package: swig-cli package-clean
    ifeq ($(ARCH),aarch64)
		cd ../ && python nic/tools/package.py
    endif

platform:
    ifeq ($(ARCH),aarch64)
		cd ../platform && $(PATH_OPTS) make $(BUILD_OPTS)
    endif

platform-clean:
	cd ../platform && make $(BUILD_OPTS) clean

capsim-master:
	cd asic/capri/model/capsim-master && $(PATH_OPTS) make $(BUILD_OPTS)

capsim-master-clean:
	cd asic/capri/model/capsim-master && make $(BUILD_OPTS) clean

capmodel:
    ifneq ($(ARCH),aarch64)
		bazel $(BZL_OPT) build $(BZL_BLD_OPT) model_sim:cap_model
		bazel $(BZL_OPT) build $(BZL_BLD_OPT) model_sim:model_client
		bazel $(BZL_OPT) build $(BZL_BLD_OPT) model_sim:model_cli
    endif

go-proto:
	$(MAKE) -C ${GOPATH}/src/github.com/pensando/sw/nic/agent/netagent/datapath/halproto

sdk:
	$(MAKE) -C sdk lib

DEL_PKGS = --deleted_packages=nic/sdk/include,nic/sdk,nic/sdk/obj
#SDK_OPTS = $(DEL_PKGS) --copt=-Wno-unused-function 
SDK_OPTS = --copt=-Wno-unused-function 

pd_iris:
	bazel $(BZL_OPT) build $(BZL_BLD_OPT) $(SDK_OPTS) hal/pd/iris:libiris.so

pd_stub:
	bazel $(BZL_OPT) build $(BZL_BLD_OPT) $(SDK_OPTS) hal/pd/pd_stub

pd_gft:
	bazel $(BZL_OPT) build $(BZL_BLD_OPT) --copt=-DGFT --define p4pipeline=gft $(SDK_OPTS) hal/pd/gft:all

p4:
	$(MAKE) -j4 -C p4

asm:
	$(MAKE) -C asm

proto:
	$(MAKE) -C proto

gen: proto
	$(MAKE) -j8 -C gen/proto/hal

hal: p4 asm gen sdk capsim-master platform
	bazel $(BZL_OPT) build $(BZL_BLD_OPT) $(SDK_OPTS) hal:hal

plugins:
	bazel $(BZL_OPT) build $(BZL_BLD_OPT) $(SDK_OPTS) hal/plugins/...

utils:
	bazel $(BZL_OPT) build $(BZL_BLD_OPT) $(SDK_OPTS) utils/host_mem:all

linkmgr:
	bazel $(BZL_OPT) build $(BZL_BLD_OPT) $(SDK_OPTS) linkmgr:linkmgr

post-make:
	mkdir -p obj/cano
	find obj -name '*.cano'  -type f  | xargs -i mv {} obj/cano 2>/dev/null || :

pd: pd_iris pd_stub

nic: hal pd plugins utils linkmgr capmodel go-proto
	$(MAKE) post-make
	$(MAKE) package

all: nic tests

clean: platform-clean capsim-master-clean
	bazel clean
	rm -rf $(VALGRIND_DIR)
	rm -rf $(GEN_TEST_RESULTS_DIR)
	rm -rf conf/hal.ini
	$(MAKE) -C p4 clean
	$(MAKE) -C asm clean
	$(MAKE) -C proto clean
	$(MAKE) -C sdk clean
	rm -rf  $(shell realpath "$(TOPDIR_ABS)"/../bazel-out/..)
	rm -rf gen/capri_loader.conf
	$(MAKE) check_clean

check_clean:
	@tools/check_clean.sh gen
