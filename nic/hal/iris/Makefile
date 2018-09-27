.PHONY: p4 asm hal utils linkmgr sdk third-party agent fte_sim delphi plugins

default: all

ifndef TOPDIR
export TOPDIR := $(realpath ${CURDIR}/../../)
endif

ifeq ($(RTL),1)
export ASMOPTS := "-DCAPRI_IGNORE_TIMESTAMP -DRTL"
BZL_BLD_OPT := $(BZL_BLD_OPT) --linkopt=-L/usr/local/lib
endif

ifeq ($(PLATFORM),haps)
export ASMOPTS := "-DHAPS"
endif

export BZL_BUILD := "bazel $(BZL_OPT) build $(BZL_BLD_OPT)"

%/.dir:
	$(Q)mkdir -p $(@D) && touch $@

include $(TOPDIR)/make/Makefile.common

#################################################
############## VALGRIND #########################
#################################################

VAL_CMD = valgrind --xml=yes
VAL_CMD_OPTS = $(CMD_OPTS) $(VAL_CMD)
VALGRIND_DIR= $(TOPDIR)/gen/valgrind
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
#################### HAL ########################
#################################################

P4PLUS_MOD_LIST = \
				  	adminq_txdma_actions			\
					cpu_rxdma						\
					cpu_txdma						\
					esp_ipv4_tunnel_h2n_rxdma		\
					esp_ipv4_tunnel_h2n_txdma1		\
					esp_ipv4_tunnel_h2n_txdma2		\
					esp_v4_tunnel_n2h_rxdma			\
					esp_v4_tunnel_n2h_txdma1		\
					esp_v4_tunnel_n2h_txdma2		\
					eth_rxdma_actions				\
					eth_txdma_actions				\
					gc_txdma						\
					ipfix							\
					p4pt							\
					proxyc_txdma					\
					proxyr_txdma					\
					rawc_txdma						\
					rawr_rxdma						\
					rdma_cq_rxdma					\
					rdma_cq_txdma					\
					rdma_req_rxdma					\
					rdma_req_txdma					\
					rdma_resp_rxdma					\
					rdma_resp_txdma					\
					rdma_rxdma						\
					smbdc_req_txdma					\
					storage_nvme					\
					storage_seq						\
					storage_tx						\
					tcp_proxy_rxdma					\
					tcp_proxy_txdma					\
					tls_txdma_post_crypto_dec		\
					tls_txdma_post_crypto_dec_aesgcm_newseg	\
					tls_txdma_post_crypto_enc		\
					tls_txdma_post_crypto_mac		\
					tls_txdma_pre_crypto_dec		\
					tls_txdma_pre_crypto_dec_aesgcm_newseg	\
					tls_txdma_pre_crypto_enc		\
					tls_txdma_pre_crypto_mac		\
					virtio_rxdma					\
					virtio_txdma					\

swig-cli-clean:
	make -C $(TOPDIR)/debug_cli clean
	make -C $(TOPDIR)/debug_cli clean PGM=common_txdma_actions
	make -C $(TOPDIR)/debug_cli clean PGM=common_rxdma_actions
	make -C $(TOPDIR)/debug_cli clean PGM=cli
	$(foreach p4mod, $(P4PLUS_MOD_LIST),  make -C $(TOPDIR)/debug_cli clean PGM=$(p4mod); )

swig-cli:
	# Disable cli  for coverage
    ifeq ($(c),0)
		PATH=$(TOOLCHAIN_PATH)/bin:$$PATH make -C $(TOPDIR)/debug_cli
		PATH=$(TOOLCHAIN_PATH)/bin:$$PATH make -C $(TOPDIR)/debug_cli PGM=common_txdma_actions
		PATH=$(TOOLCHAIN_PATH)/bin:$$PATH make -C $(TOPDIR)/debug_cli PGM=common_rxdma_actions
		PATH=$(TOOLCHAIN_PATH)/bin:$$PATH make -C $(TOPDIR)/debug_cli PGM=cli
		$(foreach p4mod, $(P4PLUS_MOD_LIST), PATH=$(TOOLCHAIN_PATH)/bin:$$PATH make -C $(TOPDIR)/debug_cli PGM=$(p4mod); )
    endif

agent_proto:
	$(MAKE) -C ${GOPATH}/src/github.com/pensando/sw/nic/agent/netagent/datapath/halproto

plugins:
	bazel $(BZL_OPT) build $(BZL_BLD_OPT) --build_tag_filters=-gtests //nic/hal/plugins/...
    ifeq ($(ARCH),aarch64)
		bazel $(BZL_OPT) build $(BZL_BLD_OPT) --build_tag_filters=sharedlib //nic/hal/tls/engine/...
    endif

pd: pd_stub
	bazel $(BZL_OPT) build $(BZL_BLD_OPT) //nic/hal/pd/iris:libiris.so

iris-c:
	bazel $(BZL_OPT) build $(BZL_BLD_OPT) //nic/hal/iris-c:iris-c
	install -m 755 $(TOPDIR)/../bazel-bin/nic/hal/iris-c/libiris-c.so $(TOPDIR)/hal/obj

# code generation stage
stage0_DEV: irisp4 p4 gen_proto utils
stage0: stage0_DEV csrlite agent_proto

# lib building stage
stage1_DEV: iris_asm asm capmodel build_proto sdk linkmgr
stage1: stage1_DEV third-party agent delphi upgrade-manager linkmgr

# app building stage
stage2_DEV: platform hal pd
stage2: stage2_DEV swig-cli hntap fte_sim iris-c

# post build stage
stage3: post-make halctl

iris_asm:
	$(MAKE) AS_DEFINES=$(ASMOPTS) -C datapath/asm/iris

irisp4:
	$(MAKE) -C datapath/p4/

nicdev:
	@echo "Available RAM $(ram_mb)"
	$(MAKE) stage0_DEV
	$(MAKE) stage1_DEV
	$(MAKE) stage2_DEV

stages:
	@echo "Available RAM $(ram_mb)"
	$(MAKE) stage0
	$(MAKE) stage1
	$(MAKE) stage2
	$(MAKE) stage3

nic: stages
	$(MAKE) package
	$(MAKE) package-size-check
	$(MAKE) aarch64-libs-check

haps: stages
	$(MAKE) package-tests
	# package x86_64 heimdall for haps debug image
	make ARCH=x86_64 -C $(TOPDIR) heimdall
	$(MAKE) package-haps-dbg

all: stages
	$(MAKE) tests
	$(MAKE) package-tests
	$(MAKE) package-sim

clean: platform-clean
	bazel clean --expunge
	rm -rf $(VALGRIND_DIR)
	rm -rf $(GEN_TEST_RESULTS_DIR)
	rm -rf conf/hal.ini
	$(MAKE) -C $(TOPDIR)/p4 clean
	$(MAKE) -C $(TOPDIR)/asm clean
	$(MAKE) -C $(TOPDIR)/proto clean
	$(MAKE) -C $(TOPDIR)/sdk clean
	rm -rf  $(shell realpath "$(TOPDIR)"/../bazel-out/..)
	rm -rf gen/capri_loader.conf
	$(MAKE) gen_clean
	$(MAKE) check_clean

check_clean:
	@$(TOPDIR)/tools/check_clean.sh gen
