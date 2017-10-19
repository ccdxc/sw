.PHONY: hal hal-clean
default: hal

TOPDIR = .
TOPDIR_ABS = ${CURDIR}
CUSTOM = hal

include $(TOPDIR)/make/Makefile.inc
include $(TOPDIR)/make/Make.inc

# get a shell with the dependencies image loaded, with the host filesystem mounted.
shell: build-runtime-image
	mkdir -p ${PWD}/../bazel-cache;
	docker run -it --rm --privileged --net=host -v ${PWD}/..:/sw -v ${PWD}/../bazel-cache:/root/.cache -w /sw/nic pensando/nic bash

# run `make gtest` with the dependencies image.
test: build-runtime-image
	docker run -it --rm --privileged -v ${PWD}/..:/sw -v ${PWD}/../bazel-cache:/root/.cache -w /sw/nic pensando/nic bash -c 'make gtest'

# run a build with the dependencies image.
build: build-runtime-image
	docker run -it --rm --privileged -v ${PWD}/..:/sw -v ${PWD}/../bazel-cache:/root/.cache -w /sw/nic pensando/nic bash -c 'make gtest'

clean-docker: build-runtime-image
	docker run -it --rm --privileged -v ${PWD}/..:/sw -v ${PWD}/../bazel-cache:/root/.cache -w /sw/nic pensando/nic bash -c 'make clean'

REGISTRY = registry.test.pensando.io:5000

build-runtime-image: install_box
	if [ "x${NO_PULL}" = "x" ]; then docker pull $(REGISTRY)/pensando/nic:1.2; fi
	cd .. && BOX_INCLUDE_ENV="NO_COPY" NO_COPY=1 box nic/box.rb

install_box:
	@if [ ! -x /usr/local/bin/box ]; then echo "Installing box, sudo is required"; curl -sSL box-builder.sh | sudo bash; fi

# make a trial dependencies image. pass RELEASE=1 or run `make deps-release` to
# do a release build.
deps: install_box
	cd .. && RELEASE=${RELEASE} BOX_INCLUDE_ENV="RELEASE" box -t '$(REGISTRY)/pensando/nic:1.2' nic/box-deps.rb

# make a release build of the dependencies image
deps-release:
	RELEASE=1 make deps

# make a shell based on the local build of the dependencies image, for testing
deps-test-shell:
	NO_PULL=1 make shell

# do a test build with a local build of the dependencies image.
deps-test-build:
	NO_PULL=1 make build


hal:
	make bazel-build
	make bazel-gtests
	make post-make

capmodel:
	bazel $(BZL_OPT) build $(BZL_BLD_OPT) model_sim:cap_model
	bazel $(BZL_OPT) build $(BZL_BLD_OPT) model_sim:model_client
	bazel $(BZL_OPT) build $(BZL_BLD_OPT) model_sim:model_cli

go-proto:
	#cd agent/netagent/datapath/halproto && make
	#make -C agent/netagent/datapath/halproto

bazel-build:
	make -C p4
	make -C asm
	make -C proto
	make -C gen/proto/hal
	bazel $(BZL_OPT) build $(BZL_BLD_OPT) hal:hal
	#bazel $(BZL_OPT) build $(BZL_BLD_OPT) //nic/proto:all
	#bazel $(BZL_OPT) build $(BZL_BLD_OPT) //protos:all
	#bazel $(BZL_OPT) build $(BZL_BLD_OPT) //nic/proto/hal:all
	bazel $(BZL_OPT) build $(BZL_BLD_OPT) utils/host_mem:all
	bazel $(BZL_OPT) build $(BZL_BLD_OPT) //dol/test/storage:storage_test
	make capmodel
	make go-proto

bazel-gtests:
	# PI gtests
	bazel $(BZL_OPT) build $(BZL_BLD_OPT) hal/test/gtests:all
	# PI Utils
	bazel $(BZL_OPT) build $(BZL_BLD_OPT) utils/bitmap:bitmap_test
	bazel $(BZL_OPT) build $(BZL_BLD_OPT) utils/host_mem:host_mem_test
	bazel $(BZL_OPT) build $(BZL_BLD_OPT) utils/indexer:indexer_test
	bazel $(BZL_OPT) build $(BZL_BLD_OPT) utils/list:list_test
	bazel $(BZL_OPT) build $(BZL_BLD_OPT) utils/pt:pt_test
	bazel $(BZL_OPT) build $(BZL_BLD_OPT) utils/shm:shmseg_test
	bazel $(BZL_OPT) build $(BZL_BLD_OPT) utils/slab:slab_test
	bazel $(BZL_OPT) build $(BZL_BLD_OPT) utils/twheel:twheel_test
	bazel $(BZL_OPT) build $(BZL_BLD_OPT) fte:fte_test
	# PD Utils
	bazel $(BZL_OPT) build $(BZL_BLD_OPT) hal/pd/utils/acl_tcam:acl_tcam_test
	bazel $(BZL_OPT) build $(BZL_BLD_OPT) hal/pd/utils/directmap:directmap_test
	bazel $(BZL_OPT) build $(BZL_BLD_OPT) hal/pd/utils/flow:flow_test
	bazel $(BZL_OPT) build $(BZL_BLD_OPT) hal/pd/utils/hash:hash_test
	bazel $(BZL_OPT) build $(BZL_BLD_OPT) hal/pd/utils/met:met_test
	bazel $(BZL_OPT) build $(BZL_BLD_OPT) hal/pd/utils/tcam:tcam_test

post-make:
	mkdir -p obj/cano
	mv obj/asm_bin/*.cano obj/cano 2>/dev/null || :
	mv obj/p4plus_bin/*.cano obj/cano 2>/dev/null || :

GEN_TEST_RESULTS_DIR = $(TOPDIR_ABS)/gen/test_results
CMD_OPTS = cd ../bazel-bin && CAPRI_MOCK_MODE=1 WS_TOP=$(TOPDIR_ABS)/../../.. HAL_CONFIG_PATH=$(TOPDIR_ABS)/conf
PD_CMD_OPTS = cd ../bazel-bin && LD_PRELOAD=$(TOPDIR_ABS)/../bazel-bin/nic/hal/pd/utils/p4pd_mock/libp4pd_mock.so HAL_CONFIG_PATH=$(TOPDIR_ABS)/conf
run-gtests: $(GEN_TEST_RESULTS_DIR)/.dir
	# PI gtests
	$(CMD_OPTS) ./nic/hal/test/gtests/acl_test --gtest_output="xml:$(GEN_TEST_RESULTS_DIR)/acl_test.xml"
	$(CMD_OPTS) ./nic/hal/test/gtests/bm_allocator_test --gtest_output="xml:$(GEN_TEST_RESULTS_DIR)/bm_allocator_test.xml"
	$(CMD_OPTS) ./nic/hal/test/gtests/buf_pool_test --gtest_output="xml:$(GEN_TEST_RESULTS_DIR)/buf_pool_test.xml"
	$(CMD_OPTS) ./nic/hal/test/gtests/cpuif_test --gtest_output="xml:$(GEN_TEST_RESULTS_DIR)/cpuif_test.xml"
	$(CMD_OPTS) ./nic/hal/test/gtests/enicif_test --gtest_output="xml:$(GEN_TEST_RESULTS_DIR)/enicif_test.xml"
	$(CMD_OPTS) ./nic/hal/test/gtests/lif_manager_test --gtest_output="xml:$(GEN_TEST_RESULTS_DIR)/lif_manager_test.xml"
	$(CMD_OPTS) ./nic/hal/test/gtests/lif_test --gtest_output="xml:$(GEN_TEST_RESULTS_DIR)/lif_test.xml"
	$(CMD_OPTS) ./nic/hal/test/gtests/policer_test --gtest_output="xml:$(GEN_TEST_RESULTS_DIR)/policer_test.xml"
	$(CMD_OPTS) ./nic/hal/test/gtests/queue_test --gtest_output="xml:$(GEN_TEST_RESULTS_DIR)/queue_test.xml"
	$(CMD_OPTS) ./nic/hal/test/gtests/session_test --gtest_output="xml:$(GEN_TEST_RESULTS_DIR)/session_test.xml"
	$(CMD_OPTS) ./nic/hal/test/gtests/tlscb_test --gtest_output="xml:$(GEN_TEST_RESULTS_DIR)/tlscb_test.xml"
	$(CMD_OPTS) ./nic/hal/test/gtests/uplinkif_test --gtest_output="xml:$(GEN_TEST_RESULTS_DIR)/uplinkif_test.xml"
	$(CMD_OPTS) ./nic/hal/test/gtests/uplinkpc_test --gtest_output="xml:$(GEN_TEST_RESULTS_DIR)/uplinkpc_test.xml"
	$(CMD_OPTS) ./nic/hal/test/gtests/internal_test --gtest_output="xml:$(GEN_TEST_RESULTS_DIR)/internal_test.xml"
	$(CMD_OPTS) ./nic/hal/test/gtests/endpoint_test --gtest_output="xml:$(GEN_TEST_RESULTS_DIR)/endpoint_test.xml"
	$(CMD_OPTS) ./nic/hal/test/gtests/tenant_test --gtest_output="xml:$(GEN_TEST_RESULTS_DIR)/tenant_test.xml"
	$(CMD_OPTS) ./nic/hal/test/gtests/l2seg_test --gtest_output="xml:$(GEN_TEST_RESULTS_DIR)/l2seg_test.xml"
	$(CMD_OPTS) ./nic/hal/test/gtests/tunnelif_test --gtest_output="xml:$(GEN_TEST_RESULTS_DIR)/tunnelif_test.xml"
	$(CMD_OPTS) ./nic/hal/test/gtests/nwsec_test --gtest_output="xml:$(GEN_TEST_RESULTS_DIR)/nwsec_test.xml"
	# PI Utils gtests
	cd ../bazel-bin && ./nic/utils/bitmap/bitmap_test --gtest_output="xml:$(GEN_TEST_RESULTS_DIR)/bitmap_test.xml"
	cd ../bazel-bin && ./nic/utils/host_mem/host_mem_test --gtest_output="xml:$(GEN_TEST_RESULTS_DIR)/host_mem_test.xml"
	cd ../bazel-bin && ./nic/utils/indexer/indexer_test --gtest_output="xml:$(GEN_TEST_RESULTS_DIR)/indexer_test.xml"
	cd ../bazel-bin && ./nic/utils/list/list_test --gtest_output="xml:$(GEN_TEST_RESULTS_DIR)/list_test.xml"
	cd ../bazel-bin && ./nic/utils/pt/pt_test --gtest_output="xml:$(GEN_TEST_RESULTS_DIR)/pt_test.xml"
	cd ../bazel-bin && ./nic/utils/shm/shmseg_test --gtest_output="xml:$(GEN_TEST_RESULTS_DIR)/shmseg_test.xml"
	cd ../bazel-bin && ./nic/utils/slab/slab_test --gtest_output="xml:$(GEN_TEST_RESULTS_DIR)/slab_test.xml"
	cd ../bazel-bin && ./nic/utils/twheel/twheel_test --gtest_output="xml:$(GEN_TEST_RESULTS_DIR)/twheel_test.xml"
	cd ../bazel-bin && ./nic/fte/fte_test --gtest_output="xml:$(GEN_TEST_RESULTS_DIR)/fte_test.xml"
	# PD Utils gtests
	$(PD_CMD_OPTS) ./nic/hal/pd/utils/acl_tcam/acl_tcam_test --gtest_output="xml:$(GEN_TEST_RESULTS_DIR)/acl_tcam_test.xml"
	$(PD_CMD_OPTS) ./nic/hal/pd/utils/tcam/tcam_test --gtest_output="xml:$(GEN_TEST_RESULTS_DIR)/tcam_test.xml"
	$(PD_CMD_OPTS) ./nic/hal/pd/utils/directmap/directmap_test --gtest_output="xml:$(GEN_TEST_RESULTS_DIR)/directmap_test.xml"
	# Removed because of randomization
	#cd ../bazel-bin && LD_PRELOAD=$(TOPDIR_ABS)/../bazel-bin/nic/hal/pd/utils/p4pd_mock/libp4pd_mock.so HAL_CONFIG_PATH=$(TOPDIR_ABS)/conf ./nic/hal/pd/utils/flow/flow_test --gtest_output="xml:$(GEN_TEST_RESULTS_DIR)/flow.xml"
	$(PD_CMD_OPTS) ./nic/hal/pd/utils/hash/hash_test --gtest_output="xml:$(GEN_TEST_RESULTS_DIR)/hash_test.xml"
	$(PD_CMD_OPTS) ./nic/hal/pd/utils/met/met_test --gtest_output="xml:$(GEN_TEST_RESULTS_DIR)/met_test.xml"

VAL_CMD = valgrind --xml=yes 
VAL_CMD_OPTS = $(CMD_OPTS) $(VAL_CMD)
VAL_TR = $(TOPDIR_ABS)/gen/valgrind/test_results
VAL_INFO = $(TOPDIR_ABS)/gen/valgrind/info
run-gtests-valgrind: $(VAL_TR)/.dir $(VAL_INFO)/.dir
	# PI gtests
	$(VAL_CMD_OPTS) --xml-file=$(VAL_INFO)/uplinkif_val.xml ./nic/hal/test/gtests/uplinkif_test --gtest_output="xml:$(VAL_TR)/uplinkif_test.xml"
	$(VAL_CMD_OPTS) --xml-file=$(VAL_INFO)/uplinkpc_val.xml ./nic/hal/test/gtests/uplinkpc_test --gtest_output="xml:$(VAL_TR)/uplinkpc_test.xml" 
	$(VAL_CMD_OPTS) --xml-file=$(VAL_INFO)/tenant_val.xml ./nic/hal/test/gtests/tenant_test --gtest_output="xml:$(VAL_TR)/tenant_test.xml" 
	$(VAL_CMD_OPTS) --xml-file=$(VAL_INFO)/acl_val.xml ./nic/hal/test/gtests/acl_test --gtest_output="xml:$(VAL_TR)/acl_test.xml"
	$(VAL_CMD_OPTS) --xml-file=$(VAL_INFO)/bm_allocator_val.xml ./nic/hal/test/gtests/bm_allocator_test --gtest_output="xml:$(VAL_TR)/bm_allocator_test.xml"
	$(VAL_CMD_OPTS) --xml-file=$(VAL_INFO)/buf_pool_val.xml ./nic/hal/test/gtests/buf_pool_test --gtest_output="xml:$(VAL_TR)/buf_pool_test.xml"
	$(VAL_CMD_OPTS) --xml-file=$(VAL_INFO)/cpuif_val.xml ./nic/hal/test/gtests/cpuif_test --gtest_output="xml:$(VAL_TR)/cpuif_test.xml"
	$(VAL_CMD_OPTS) --xml-file=$(VAL_INFO)/enicif_val.xml ./nic/hal/test/gtests/enicif_test --gtest_output="xml:$(VAL_TR)/enicif_test.xml"
	$(VAL_CMD_OPTS) --xml-file=$(VAL_INFO)/lif_manager_val.xml ./nic/hal/test/gtests/lif_manager_test --gtest_output="xml:$(VAL_TR)/lif_manager_test.xml" 
	$(VAL_CMD_OPTS) --xml-file=$(VAL_INFO)/lif_val.xml ./nic/hal/test/gtests/lif_test --gtest_output="xml:$(VAL_TR)/lif_test.xml"
	$(VAL_CMD_OPTS) --xml-file=$(VAL_INFO)/policer_val.xml ./nic/hal/test/gtests/policer_test --gtest_output="xml:$(VAL_TR)/policer_test.xml"
	$(VAL_CMD_OPTS) --xml-file=$(VAL_INFO)/queue_val.xml ./nic/hal/test/gtests/queue_test --gtest_output="xml:$(VAL_TR)/queue_test.xml"
	$(VAL_CMD_OPTS) --xml-file=$(VAL_INFO)/session_val.xml ./nic/hal/test/gtests/session_test --gtest_output="xml:$(VAL_TR)/session_test.xml"
	$(VAL_CMD_OPTS) --xml-file=$(VAL_INFO)/tlscb_val.xml ./nic/hal/test/gtests/tlscb_test --gtest_output="xml:$(VAL_TR)/tlscb_test.xml"
	$(VAL_CMD_OPTS) --xml-file=$(VAL_INFO)/internal_val.xml ./nic/hal/test/gtests/internal_test --gtest_output="xml:$(VAL_TR)/internal_test.xml"
	$(VAL_CMD_OPTS) --xml-file=$(VAL_INFO)/endpoint_val.xml ./nic/hal/test/gtests/endpoint_test --gtest_output="xml:$(VAL_TR)/endpoint_test.xml"
	$(VAL_CMD_OPTS) --xml-file=$(VAL_INFO)/l2seg_val.xml ./nic/hal/test/gtests/l2seg_test --gtest_output="xml:$(VAL_TR)/l2seg_test.xml"
	$(VAL_CMD_OPTS) --xml-file=$(VAL_INFO)/tunnelif_val.xml ./nic/hal/test/gtests/tunnelif_test --gtest_output="xml:$(VAL_TR)/tunnelif_test.xml"
	$(VAL_CMD_OPTS) --xml-file=$(VAL_INFO)/nwsec_val.xml ./nic/hal/test/gtests/nwsec_test --gtest_output="xml:$(VAL_TR)/nwsec_test.xml"

clean:
	$(MAKE) -C p4 clean
	$(MAKE) -C asm clean
