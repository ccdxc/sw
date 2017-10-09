.PHONY: hal hal-clean
default: hal

TOPDIR = .
TOPDIR_ABS = ${CURDIR}
CUSTOM = hal
#LIBDIRS = \
#		  utils \
#		  hal/pd/capri/csr \
#		  proto gen/protobuf \
#		  p4 \
#		  capsim \
#		  asm \
#		  model_sim/build \
#		  fte \
#		  hal/svc \
#		  hal/lkl \
#		  hal/src \
#		  hal/periodic \
#		  hal/pd/iris/p4pd \
#		  gen/iris/src \
#		  gen/common_rxdma_actions/src \
#		  gen/common_txdma_actions/src \
#		  hal/plugins \
#		  hal/pd/common \
#		  hal/pd/iris \
#		  hal/pd/capri \
#		  hal/pd/utils/directmap\
#		  hal/pd/utils/flow \
#		  hal/pd/utils/hash \
#		  hal/pd/utils/met \
#		  hal/pd/utils/tcam \
#		  hal/pd/utils/acl_tcam \
#		  hal/pd/utils \
#		  hal/pd/utils/p4pd_mock \
#		  hal/test/utils \
#		  $(NULL)
#
#PGMDIRS = \
#		  hal \
#		  hal/test \
#		  fte/test \
#		  hal/test/gtests \
#		  utils/indexer/test \
#		  utils/shm/test \
#		  utils/list/test \
#		  utils/pt/test \
#		  utils/slab/test \
#		  utils/twheel/test \
#		  utils/host_mem/test \
#		  hal/pd/utils/directmap/test \
#		  hal/pd/utils/flow/test \
#		  hal/pd/utils/hash/test \
#		  hal/pd/utils/met/test \
#		  hal/pd/utils/tcam/test \
#		  hal/pd/utils/acl_tcam/test \
#		  hal/pd/iris/test/rw_tbl_lib \
#		  hal/test \
#		  agents \
#		  $(NULL)
#SUBDIRS = $(LIBDIRS) $(PGMDIRS)

include $(TOPDIR)/make/Makefile.inc
include $(TOPDIR)/make/Make.inc

# get a shell with the dependencies image loaded, with the host filesystem mounted.
shell: build-runtime-image
	mkdir -p ${PWD}/../bazel-cache;
	docker run -it --privileged --net=host -v ${PWD}/..:/sw -v ${PWD}/../bazel-cache:/root/.cache -w /sw/nic pensando/nic bash

# run `make gtest` with the dependencies image.
test: build-runtime-image
	docker run -it --privileged -v ${PWD}/..:/sw -v ${PWD}/../bazel-cache:/root/.cache -w /sw/nic pensando/nic bash -c 'make gtest'

# run a build with the dependencies image.
build: build-runtime-image
	docker run -it --privileged -v ${PWD}/..:/sw -v ${PWD}/../bazel-cache:/root/.cache -w /sw/nic pensando/nic bash -c 'make gtest'

clean-docker: build-runtime-image
	docker run -it --privileged -v ${PWD}/..:/sw -v ${PWD}/../bazel-cache:/root/.cache -w /sw/nic pensando/nic bash -c 'make clean'

REGISTRY = registry.test.pensando.io:5000

build-runtime-image: install_box
	if [ "x${NO_PULL}" = "x" ]; then docker pull $(REGISTRY)/pensando/nic:1.1; fi
	BOX_INCLUDE_ENV="NO_COPY" NO_COPY=1 box box.rb

install_box:
	@if [ ! -x /usr/local/bin/box ]; then echo "Installing box, sudo is required"; curl -sSL box-builder.sh | sudo bash; fi

# make a trial dependencies image. pass RELEASE=1 or run `make deps-release` to
# do a release build.
deps: install_box
	RELEASE=${RELEASE} BOX_INCLUDE_ENV="RELEASE" box -t '$(REGISTRY)/pensando/nic:1.1' box-deps.rb

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

capmodel:
	bazel $(BZL_OPT) build $(BZL_BLD_OPT) model_sim:cap_model
	bazel $(BZL_OPT) build $(BZL_BLD_OPT) model_sim:model_client
	bazel $(BZL_OPT) build $(BZL_BLD_OPT) model_sim:model_cli

bazel-build:
	make -C p4
	make -C asm
	bazel $(BZL_OPT) build $(BZL_BLD_OPT) hal:hal
	bazel $(BZL_OPT) build $(BZL_BLD_OPT) //nic/proto:all
	bazel $(BZL_OPT) build $(BZL_BLD_OPT) //protos:all
	bazel $(BZL_OPT) build $(BZL_BLD_OPT) //nic/proto/hal:all
	bazel $(BZL_OPT) build $(BZL_BLD_OPT) utils/host_mem:all
	bazel $(BZL_OPT) build $(BZL_BLD_OPT) //dol/test/storage:storage_test
	make capmodel

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

GEN_TEST_RESULTS_DIR = $(TOPDIR_ABS)/gen/test_results
run-gtests:
	# PI gtests
	cd ../bazel-bin && CAPRI_MOCK_MODE=1 WS_TOP=$(TOPDIR_ABS)/../../.. HAL_CONFIG_PATH=$(TOPDIR_ABS)/conf ./nic/hal/test/gtests/acl_test --gtest_output="xml:$(GEN_TEST_RESULTS_DIR)/acl_test.xml"
	cd ../bazel-bin && CAPRI_MOCK_MODE=1 WS_TOP=$(TOPDIR_ABS)/../../.. HAL_CONFIG_PATH=$(TOPDIR_ABS)/conf ./nic/hal/test/gtests/bm_allocator_test --gtest_output="xml:$(GEN_TEST_RESULTS_DIR)/bm_allocator_test.xml"
	cd ../bazel-bin && CAPRI_MOCK_MODE=1 WS_TOP=$(TOPDIR_ABS)/../../.. HAL_CONFIG_PATH=$(TOPDIR_ABS)/conf ./nic/hal/test/gtests/buf_pool_test --gtest_output="xml:$(GEN_TEST_RESULTS_DIR)/buf_pool_test.xml"
	cd ../bazel-bin && CAPRI_MOCK_MODE=1 WS_TOP=$(TOPDIR_ABS)/../../.. HAL_CONFIG_PATH=$(TOPDIR_ABS)/conf ./nic/hal/test/gtests/cpuif_test --gtest_output="xml:$(GEN_TEST_RESULTS_DIR)/cpuif_test.xml"
	cd ../bazel-bin && CAPRI_MOCK_MODE=1 WS_TOP=$(TOPDIR_ABS)/../../.. HAL_CONFIG_PATH=$(TOPDIR_ABS)/conf ./nic/hal/test/gtests/enicif_test --gtest_output="xml:$(GEN_TEST_RESULTS_DIR)/enicif_test.xml"
	cd ../bazel-bin && CAPRI_MOCK_MODE=1 WS_TOP=$(TOPDIR_ABS)/../../.. HAL_CONFIG_PATH=$(TOPDIR_ABS)/conf ./nic/hal/test/gtests/lif_manager_test --gtest_output="xml:$(GEN_TEST_RESULTS_DIR)/lif_manager_test.xml"
	cd ../bazel-bin && CAPRI_MOCK_MODE=1 WS_TOP=$(TOPDIR_ABS)/../../.. HAL_CONFIG_PATH=$(TOPDIR_ABS)/conf ./nic/hal/test/gtests/lif_test --gtest_output="xml:$(GEN_TEST_RESULTS_DIR)/lif_test.xml"
	cd ../bazel-bin && CAPRI_MOCK_MODE=1 WS_TOP=$(TOPDIR_ABS)/../../.. HAL_CONFIG_PATH=$(TOPDIR_ABS)/conf ./nic/hal/test/gtests/policer_test --gtest_output="xml:$(GEN_TEST_RESULTS_DIR)/policer_test.xml"
	cd ../bazel-bin && CAPRI_MOCK_MODE=1 WS_TOP=$(TOPDIR_ABS)/../../.. HAL_CONFIG_PATH=$(TOPDIR_ABS)/conf ./nic/hal/test/gtests/queue_test --gtest_output="xml:$(GEN_TEST_RESULTS_DIR)/queue_test.xml"
	cd ../bazel-bin && CAPRI_MOCK_MODE=1 WS_TOP=$(TOPDIR_ABS)/../../.. HAL_CONFIG_PATH=$(TOPDIR_ABS)/conf ./nic/hal/test/gtests/session_test --gtest_output="xml:$(GEN_TEST_RESULTS_DIR)/session_test.xml"
	#cd ../bazel-bin && CAPRI_MOCK_MODE=1 WS_TOP=$(TOPDIR_ABS)/../../.. HAL_CONFIG_PATH=$(TOPDIR_ABS)/conf ./nic/hal/test/gtests/tcpcb_test --gtest_output="xml:$(GEN_TEST_RESULTS_DIR)/tcpcb_test.xml"
	cd ../bazel-bin && CAPRI_MOCK_MODE=1 WS_TOP=$(TOPDIR_ABS)/../../.. HAL_CONFIG_PATH=$(TOPDIR_ABS)/conf ./nic/hal/test/gtests/tlscb_test --gtest_output="xml:$(GEN_TEST_RESULTS_DIR)/tlscb_test.xml"
	cd ../bazel-bin && CAPRI_MOCK_MODE=1 WS_TOP=$(TOPDIR_ABS)/../../.. HAL_CONFIG_PATH=$(TOPDIR_ABS)/conf ./nic/hal/test/gtests/uplinkif_test --gtest_output="xml:$(GEN_TEST_RESULTS_DIR)/uplinkif_test.xml"
	cd ../bazel-bin && CAPRI_MOCK_MODE=1 WS_TOP=$(TOPDIR_ABS)/../../.. HAL_CONFIG_PATH=$(TOPDIR_ABS)/conf ./nic/hal/test/gtests/uplinkpc_test --gtest_output="xml:$(GEN_TEST_RESULTS_DIR)/uplinkpc_test.xml"
	cd ../bazel-bin && CAPRI_MOCK_MODE=1 WS_TOP=$(TOPDIR_ABS)/../../.. HAL_CONFIG_PATH=$(TOPDIR_ABS)/conf ./nic/hal/test/gtests/internal_test --gtest_output="xml:$(GEN_TEST_RESULTS_DIR)/internal_test.xml"
	cd ../bazel-bin && CAPRI_MOCK_MODE=1 WS_TOP=$(TOPDIR_ABS)/../../.. HAL_CONFIG_PATH=$(TOPDIR_ABS)/conf ./nic/hal/test/gtests/endpoint_test --gtest_output="xml:$(GEN_TEST_RESULTS_DIR)/endpoint_test.xml"
	cd ../bazel-bin && CAPRI_MOCK_MODE=1 WS_TOP=$(TOPDIR_ABS)/../../.. HAL_CONFIG_PATH=$(TOPDIR_ABS)/conf ./nic/hal/test/gtests/tenant_test --gtest_output="xml:$(GEN_TEST_RESULTS_DIR)/tenant_test.xml"
	cd ../bazel-bin && CAPRI_MOCK_MODE=1 WS_TOP=$(TOPDIR_ABS)/../../.. HAL_CONFIG_PATH=$(TOPDIR_ABS)/conf ./nic/hal/test/gtests/l2seg_test --gtest_output="xml:$(GEN_TEST_RESULTS_DIR)/l2seg_test.xml"
	cd ../bazel-bin && CAPRI_MOCK_MODE=1 WS_TOP=$(TOPDIR_ABS)/../../.. HAL_CONFIG_PATH=$(TOPDIR_ABS)/conf ./nic/hal/test/gtests/tunnelif_test --gtest_output="xml:$(GEN_TEST_RESULTS_DIR)/tunnelif_test.xml"
	cd ../bazel-bin && CAPRI_MOCK_MODE=1 WS_TOP=$(TOPDIR_ABS)/../../.. HAL_CONFIG_PATH=$(TOPDIR_ABS)/conf ./nic/hal/test/gtests/nwsec_test --gtest_output="xml:$(GEN_TEST_RESULTS_DIR)/nwsec_test.xml"
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
	cd ../bazel-bin && LD_PRELOAD=$(TOPDIR_ABS)/../bazel-bin/nic/hal/pd/utils/p4pd_mock/libp4pd_mock.so HAL_CONFIG_PATH=$(TOPDIR_ABS)/conf ./nic/hal/pd/utils/acl_tcam/acl_tcam_test --gtest_output="xml:$(GEN_TEST_RESULTS_DIR)/acl_tcam_test.xml"
	cd ../bazel-bin && LD_PRELOAD=$(TOPDIR_ABS)/../bazel-bin/nic/hal/pd/utils/p4pd_mock/libp4pd_mock.so HAL_CONFIG_PATH=$(TOPDIR_ABS)/conf ./nic/hal/pd/utils/tcam/tcam_test --gtest_output="xml:$(GEN_TEST_RESULTS_DIR)/tcam_test.xml"
	cd ../bazel-bin && LD_PRELOAD=$(TOPDIR_ABS)/../bazel-bin/nic/hal/pd/utils/p4pd_mock/libp4pd_mock.so HAL_CONFIG_PATH=$(TOPDIR_ABS)/conf ./nic/hal/pd/utils/directmap/directmap_test --gtest_output="xml:$(GEN_TEST_RESULTS_DIR)/directmap_test.xml"
	# Removed because of randomization
	#cd ../bazel-bin && LD_PRELOAD=$(TOPDIR_ABS)/../bazel-bin/nic/hal/pd/utils/p4pd_mock/libp4pd_mock.so HAL_CONFIG_PATH=$(TOPDIR_ABS)/conf ./nic/hal/pd/utils/flow/flow_test --gtest_output="xml:$(GEN_TEST_RESULTS_DIR)/flow.xml"
	cd ../bazel-bin && LD_PRELOAD=$(TOPDIR_ABS)/../bazel-bin/nic/hal/pd/utils/p4pd_mock/libp4pd_mock.so HAL_CONFIG_PATH=$(TOPDIR_ABS)/conf ./nic/hal/pd/utils/hash/hash_test --gtest_output="xml:$(GEN_TEST_RESULTS_DIR)/hash_test.xml"
	cd ../bazel-bin && LD_PRELOAD=$(TOPDIR_ABS)/../bazel-bin/nic/hal/pd/utils/p4pd_mock/libp4pd_mock.so HAL_CONFIG_PATH=$(TOPDIR_ABS)/conf ./nic/hal/pd/utils/met/met_test --gtest_output="xml:$(GEN_TEST_RESULTS_DIR)/met_test.xml"

clean:
	$(MAKE) -C p4 clean
	$(MAKE) -C asm clean
