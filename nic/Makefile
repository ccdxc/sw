.PHONY: p4 asm hal utils linkmgr sdk third-party agent

default: all

TOPDIR = .
TOPDIR_ABS = ${CURDIR}

%/.dir:
	$(Q)mkdir -p $(@D) && touch $@

include $(TOPDIR)/make/Make.inc
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
hal-rtl: p4 platform capsim-master
	$(MAKE) AS_DEFINES="-DCAPRI_IGNORE_TIMESTAMP -DRTL" asm
	$(MAKE) gen-proto
	$(MAKE) build-proto
	$(MAKE) third-party
	$(MAKE) csrlite
	$(MAKE) sdk
	bazel $(BZL_OPT) build $(BZL_NON_DOCK_BLD_OPT) $(SDK_OPTS) hal:hal
	bazel $(BZL_OPT) build $(BZL_NON_DOCK_BLD_OPT) $(SDK_OPTS) hal/pd/iris:libiris.so
	bazel $(BZL_OPT) build $(BZL_NON_DOCK_BLD_OPT) $(SDK_OPTS) hal/pd/pd_stub
	bazel $(BZL_OPT) build $(BZL_NON_DOCK_BLD_OPT) $(SDK_OPTS) --build_tag_filters=-gtests hal/plugins/...
	bazel $(BZL_OPT) build $(BZL_NON_DOCK_BLD_OPT) $(SDK_OPTS) //dol/test/storage:storage_test
	$(MAKE) utils
	$(MAKE) capmodel

#################################################
################## GFT ##########################
#################################################

swig-cli-gft-clean:
	make -C debug_cli clean PGM=gft
	make -C debug_cli clean PGM=common_txdma_actions
	make -C debug_cli clean PGM=common_rxdma_actions
	make -C debug_cli clean PGM=cli

swig-cli-gft: swig-cli-gft-clean
	PATH=$(TOOLCHAIN_PATH)/bin:$$PATH make -C debug_cli PGM=gft
	PATH=$(TOOLCHAIN_PATH)/bin:$$PATH make -C debug_cli PGM=common_txdma_actions
	PATH=$(TOOLCHAIN_PATH)/bin:$$PATH make -C debug_cli PGM=common_rxdma_actions
	PATH=$(TOOLCHAIN_PATH)/bin:$$PATH make -C debug_cli PGM=cli

gft: platform capsim-master go-proto
	$(MAKE) capmodel
	$(MAKE) -j4 -C p4 msft_gft
	$(MAKE) -C asm msft_gft AS_DEFINES="-DGFT -DCAPRI_IGNORE_TIMESTAMP"
	$(MAKE) gen-proto
	$(MAKE) -j4 build-proto
	$(MAKE) csrlite
	$(MAKE) sdk
	bazel $(BZL_OPT) build $(BZL_BLD_OPT) $(SDK_OPTS) hal:hal
	#bazel build --copt=-DGFT $(SDK_OPTS) hal:hal
	$(MAKE) pd_gft
	$(MAKE) pd_stub
	$(MAKE) gtests_gft
	$(MAKE) swig-cli-gft
	bazel build --copt=-DGFT --define p4pipeline=gft $(SDK_OPTS) hal/test/gtests:gft_test
	bazel $(BZL_OPT) build $(BZL_BLD_OPT) $(SDK_OPTS) utils/host_mem:all
	$(MAKE) post-make

gtests_gft:
	bazel build --copt=-DGFT --define p4pipeline=gft $(SDK_OPTS) hal/test/gtests:lif_gft_test
	bazel build --copt=-DGFT --define p4pipeline=gft $(SDK_OPTS) hal/test/gtests:uplinkif_gft_test
	bazel build --copt=-DGFT --define p4pipeline=gft $(SDK_OPTS) hal/test/gtests:enicif_gft_test
	bazel build --copt=-DGFT --define p4pipeline=gft $(SDK_OPTS) hal/test/gtests:endpoint_gft_test
	bazel build --copt=-DGFT --define p4pipeline=gft $(SDK_OPTS) hal/test/gtests:emp_gft_test
	bazel build --copt=-DGFT --define p4pipeline=gft $(SDK_OPTS) hal/test/gtests:efe_gft_test
	 @# Once PD is properly cleaned up gft_test should be compiled with the below cmd. Remove above cmds
	#bazel build $(SDK_OPTS) --build_tag_filters=gft hal/test/gtests:all

run-gft-gtests: $(GEN_TEST_RESULTS_DIR)/.dir
	$(CMD_OPTS) ./nic/hal/test/gtests/lif_gft_test --gtest_output="xml:$(GEN_TEST_RESULTS_DIR)/lif_gft_test.xml"
	$(CMD_OPTS) ./nic/hal/test/gtests/uplinkif_gft_test --gtest_output="xml:$(GEN_TEST_RESULTS_DIR)/uplinkif_gft_test.xml"
	$(CMD_OPTS) ./nic/hal/test/gtests/enicif_gft_test --gtest_output="xml:$(GEN_TEST_RESULTS_DIR)/enicif_gft_test.xml"
	$(CMD_OPTS) ./nic/hal/test/gtests/endpoint_gft_test --gtest_output="xml:$(GEN_TEST_RESULTS_DIR)/endpoint_gft_test.xml"
	$(CMD_OPTS) ./nic/hal/test/gtests/emp_gft_test --gtest_output="xml:$(GEN_TEST_RESULTS_DIR)/emp_gft_test.xml"
	$(CMD_OPTS) ./nic/hal/test/gtests/efe_gft_test --gtest_output="xml:$(GEN_TEST_RESULTS_DIR)/efe_gft_test.xml"

####################################################
#################### Oracle ########################
####################################################

tyche:
	$(MAKE) -j4 -C p4 tyche

####################################################
#################### DELPHI ########################
####################################################

delphi:
	bazel build //nic/delphi/...

###########################################################
#################### UPGRADE Infra ########################
###########################################################

upgrade-manager:
	bazel build //nic/upgrade_manager/...
	bazel build //nic/moveable_objects/...

#################################################
#################### HAL ########################
#################################################

swig-cli-clean:
	make -C debug_cli clean
	make -C debug_cli clean PGM=common_txdma_actions
	make -C debug_cli clean PGM=common_rxdma_actions
	make -C debug_cli clean PGM=cli

swig-cli: swig-cli-clean
	PATH=$(TOOLCHAIN_PATH)/bin:$$PATH make -C debug_cli
	PATH=$(TOOLCHAIN_PATH)/bin:$$PATH make -C debug_cli PGM=common_txdma_actions
	PATH=$(TOOLCHAIN_PATH)/bin:$$PATH make -C debug_cli PGM=common_rxdma_actions
	PATH=$(TOOLCHAIN_PATH)/bin:$$PATH make -C debug_cli PGM=cli

package-size-check: package
    ifeq ($(ARCH),aarch64)
	$(eval FILE_NAME = nic.tar)
	# max file size is 146MB
	$(eval MAX_FILE_SIZE = 153092096)
	$(eval FILE_SIZE=$(shell stat -L -c %s $(FILE_NAME)))

	if [ $(FILE_SIZE) -gt $(MAX_FILE_SIZE) ] ; then \
		echo $(FILE_NAME) size is $(FILE_SIZE). Expected to be less than $(MAX_FILE_SIZE);  \
		exit 1; \
	fi
    endif

package-clean:
	@cd ../ && rm -rf fake_root_target
	@rm -f nic.tgz
	@rm -f nic.tar

package: package-clean
    ifeq ($(ARCH),aarch64)
		cd ../ && python nic/tools/package.py
    else
		cd ../ && python nic/tools/package.py x86_64
    endif

release:
    ifneq ($(ARCH),aarch64)
	    ./tools/release.sh
    endif

release-clean:
	@rm -rf obj/release

platform:
    ifeq ($(ARCH),aarch64)
		cd ../platform && $(PATH_OPTS) make $(BUILD_OPTS)
    endif

platform-clean:
	cd ../platform && make $(BUILD_OPTS) clean

capsim-master:
	cd asic/capri/model/capsim-master && $(MAKE)
	cd asic/capri/model/capsim-master && $(PATH_OPTS) make $(BUILD_OPTS)

capsim-master-clean:
	cd asic/capri/model/capsim-master && make $(BUILD_OPTS) clean

capmodel:
    ifneq ($(ARCH),aarch64)
		bazel $(BZL_OPT) build $(BZL_BLD_OPT) model_sim:cap_model
		bazel $(BZL_OPT) build $(BZL_BLD_OPT) model_sim:model_client
		bazel $(BZL_OPT) build $(BZL_BLD_OPT) model_sim:model_cli
    endif

hntap:
    ifneq ($(ARCH),aarch64)
		bazel $(BZL_OPT) build $(BZL_BLD_OPT) $(SDK_OPTS) //nic/e2etests:nic_infra_hntap
    endif

go-proto:
	$(MAKE) -C ${GOPATH}/src/github.com/pensando/sw/nic/agent/netagent/datapath/halproto

sdk:
	$(MAKE) -C sdk lib

DEL_PKGS = --deleted_packages=nic/sdk/include,nic/sdk,nic/sdk/obj
#SDK_OPTS = $(DEL_PKGS) --copt=-Wno-unused-function
#SDK_OPTS = --copt=-Wno-unused-function

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

gen-clean:
	@rm -rf gen/x86_64
	@rm -rf gen/aarch64
	@rm -rf gen/common
	$(MAKE) clean-svc
	$(MAKE) clean-csrlite

gen-proto:
	$(MAKE) -C proto
	$(MAKE) gen-svc

build-proto:
	$(PATH_OPTS) $(MAKE) -j8 $(BUILD_OPTS) -C gen/proto/hal

##### HAL SVC #####

PROTOSRCDIR := proto/hal
HALGENDIR   := gen/hal
SVCGENDIR   := $(HALGENDIR)/svc

$(SVCGENDIR)/.dir: $(PROTOSRCDIR)/*.proto
	@mkdir -p $(SVCGENDIR)
	@touch $@
	@python3 ./tools/hal/hal_svc_generator.py

gen-svc: $(SVCGENDIR)/.dir

clean-svc:
	@rm -rf $(HALGENDIR)

##### CSRLITE #####

CSRLITESRCDIR := asic/capri/verif/common/csr_gen
CSRLITEGENDIR := gen/csrlite

$(CSRLITEGENDIR)/.dir: $(CSRLITESRCDIR)/*.yaml
	@mkdir -p $(CSRLITEGENDIR)
	@touch $@
	@cd tools && python gen_cap_csrlite.py
	@cp asic/capri/model/cap_top/cap_top_csr_defines.h sdk/include/sdk/asic/capri/csrlite/

gen-csrlite: $(CSRLITEGENDIR)/.dir

clean-csrlite:
	@rm -rf $(CSRLITEGENDIR)

build-csrlite:
	bazel $(BZL_OPT) build $(BZL_BLD_OPT) $(SDK_OPTS) hal/pd/capri/csrlite:libcsrlite.so

csrlite:
	$(MAKE) gen-csrlite
	$(MAKE) build-csrlite

# make target to run bazel build with appropriate build OPTS
# usage: make bazel-build target=fte:fte_test
bazel-build:
	bazel $(BZL_OPT) build $(BZL_BLD_OPT) $(SDK_OPTS) $(target)

hal:
    ifeq ($(PROFILING),1)
		bazel $(BZL_OPT) build $(BZL_BLD_OPT) $(SDK_OPTS) hal:hal_perf
    else
		bazel $(BZL_OPT) build $(BZL_BLD_OPT) $(SDK_OPTS) hal:hal
    endif

third-party:
	bazel $(BZL_OPT) build $(BZL_BLD_OPT) $(SDK_OPTS) hal/third-party/isc-dhcp:isc-dhcp
	cp $(TOPDIR_ABS)/../bazel-bin/nic/hal/third-party/isc-dhcp/libisc-dhcp.so $(TOPDIR_ABS)/hal/obj
	chmod +w "$(TOPDIR_ABS)/hal/obj/libisc-dhcp.so"

plugins:
    ifeq ($(ARCH),aarch64)
		bazel $(BZL_OPT) build $(BZL_BLD_OPT) $(SDK_OPTS) --build_tag_filters=-gtests hal/plugins/...
    else
		bazel $(BZL_OPT) build $(BZL_BLD_OPT) $(SDK_OPTS) hal/plugins/...
    endif

utils:
	bazel $(BZL_OPT) build $(BZL_BLD_OPT) $(SDK_OPTS) utils/host_mem:all

linkmgr:
	bazel $(BZL_OPT) build $(BZL_BLD_OPT) $(SDK_OPTS) linkmgr:linkmgr

post-make:
	mkdir -p obj/cano
	find obj -name '*.cano'  -type f  | xargs -i mv {} obj/cano 2>/dev/null || :

halctl:
	# Disable halctl for coverage
    ifeq ($(c),0)
		$(MAKE) -C ${GOPATH}/src/github.com/pensando/sw/nic/agent/cmd/halctl
		# need to copy to nic/obj for packaging
		cp ${GOPATH}/bin/halctl obj/
    endif

pd: pd_iris pd_stub

agent:
    ifneq ($(ARCH),aarch64)
    ifeq ($(c),0)
	@cd ../ && mkdir -p nic/obj/agent
	@cd ../ && go build -o nic/obj/agent/netagent github.com/pensando/sw/nic/agent/cmd/netagent
    endif
    endif

# code generation stage
stage0: p4 gen-proto csrlite go-proto utils capsim-master

# lib building stage
stage1: asm capmodel build-proto sdk third-party agent delphi

# app building stage
stage2: plugins linkmgr platform pd hal swig-cli hntap

# post build stage
stage3: post-make package package-size-check halctl upgrade-manager

nic:
	$(MAKE) stage0
	$(MAKE) stage1
	$(MAKE) stage2
	$(MAKE) stage3

all:    nic
	$(MAKE) tests
	$(MAKE) package-tests

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
	$(MAKE) gen-clean
	$(MAKE) check_clean

check_clean:
	@tools/check_clean.sh gen

check_traces:
	@./tools/trace_valid.py

sanity_logs:
	tar cvzf nic_sanity_logs.tar.gz -T /dev/null  \
        $(shell ls                          \
                core.hal                    \
                core.capmodel               \
                gen/capri_loader.conf       \
                /root/naples/data/logs/*    \
                *.log                       \
                *.log.*                     \
                ../dol/*.log                \
                ../dol/*.log.*)
