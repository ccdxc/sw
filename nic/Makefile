

TOPDIR = .
LIBDIRS = \
		  utils \
		  hal/pd/capri/csr \
		  proto gen/protobuf \
		  p4 \
		  capsim \
		  asm \
		  model_sim/build \
		  fte \
		  hal/svc \
		  hal/lkl \
		  hal/src \
		  hal/periodic \
		  hal/pd/iris/p4pd \
		  gen/iris/src \
		  gen/common_rxdma_actions/src \
		  gen/common_txdma_actions/src \
		  hal/plugins \
		  hal/pd/common \
		  hal/pd/iris \
		  hal/pd/capri \
		  hal/pd/utils/directmap\
		  hal/pd/utils/flow \
		  hal/pd/utils/hash \
		  hal/pd/utils/met \
		  hal/pd/utils/tcam \
		  hal/pd/utils/acl_tcam \
		  hal/pd/utils \
		  hal/pd/utils/p4pd_mock \
		  hal/test/utils \
		  $(NULL)

PGMDIRS = \
		  hal \
		  hal/test \
		  fte/test \
		  hal/test/gtests \
		  utils/indexer/test \
		  utils/shm/test \
		  utils/list/test \
		  utils/pt/test \
		  utils/slab/test \
		  utils/twheel/test \
		  utils/host_mem/test \
		  hal/pd/utils/directmap/test \
		  hal/pd/utils/flow/test \
		  hal/pd/utils/hash/test \
		  hal/pd/utils/met/test \
		  hal/pd/utils/tcam/test \
		  hal/pd/utils/acl_tcam/test \
		  hal/pd/iris/test/rw_tbl_lib \
		  hal/test \
		  agents \
		  $(NULL)
SUBDIRS = $(LIBDIRS) $(PGMDIRS)

include $(TOPDIR)/make/Makefile.inc

# get a shell with the dependencies image loaded, with the host filesystem mounted.
shell: build-runtime-image
	mkdir -p ${PWD}/../bazel-cache;
	docker run -it --privileged -v ${PWD}/..:/sw -v ${PWD}/../bazel-cache:/root/.cache -w /sw/nic pensando/nic bash

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

bazel-build:
	make -j4 -C p4
	make -j4 -C asm
	bazel build hal:hal
	bazel build model_sim:cap_model
	bazel build //nic/proto:all
	bazel build //nic/proto/hal:all
	bazel build //nic/proto/agents:all
	bazel build utils/host_mem:all

bazel-gtests:
	# PI gtests
	bazel build hal/test/gtests:all
	# PI Utils
	bazel build utils/bitmap:bitmap_test
	bazel build utils/host_mem:host_mem_test
	bazel build utils/indexer:indexer_test
	bazel build utils/list:list_test
	bazel build utils/pt:pt_test
	bazel build utils/shm:shmseg_test
	bazel build utils/slab:slab_test
	bazel build utils/twheel:twheel_test
	# PD Utils
	bazel build hal/pd/utils/acl_tcam:acl_tcam_test
	bazel build hal/pd/utils/directmap:directmap_test
	bazel build hal/pd/utils/flow:flow_test
	bazel build hal/pd/utils/hash:hash_test
	bazel build hal/pd/utils/met:met_test
	bazel build hal/pd/utils/tcam:tcam_test

run-gtests:
	# PI gtests
	CAPRI_MOCK_MODE=1 WS_TOP=$(TOPDIR)/../../.. HAL_CONFIG_PATH=$(TOPDIR)/conf ../bazel-bin/nic/hal/test/gtests/acl_test
	CAPRI_MOCK_MODE=1 WS_TOP=$(TOPDIR)/../../.. HAL_CONFIG_PATH=$(TOPDIR)/conf ../bazel-bin/nic/hal/test/gtests/bm_allocator_test
	CAPRI_MOCK_MODE=1 WS_TOP=$(TOPDIR)/../../.. HAL_CONFIG_PATH=$(TOPDIR)/conf ../bazel-bin/nic/hal/test/gtests/buf_pool_test
	CAPRI_MOCK_MODE=1 WS_TOP=$(TOPDIR)/../../.. HAL_CONFIG_PATH=$(TOPDIR)/conf ../bazel-bin/nic/hal/test/gtests/cpuif_test
	CAPRI_MOCK_MODE=1 WS_TOP=$(TOPDIR)/../../.. HAL_CONFIG_PATH=$(TOPDIR)/conf ../bazel-bin/nic/hal/test/gtests/enicif_test
	CAPRI_MOCK_MODE=1 WS_TOP=$(TOPDIR)/../../.. HAL_CONFIG_PATH=$(TOPDIR)/conf ../bazel-bin/nic/hal/test/gtests/lif_manager_test
	CAPRI_MOCK_MODE=1 WS_TOP=$(TOPDIR)/../../.. HAL_CONFIG_PATH=$(TOPDIR)/conf ../bazel-bin/nic/hal/test/gtests/lif_test
	CAPRI_MOCK_MODE=1 WS_TOP=$(TOPDIR)/../../.. HAL_CONFIG_PATH=$(TOPDIR)/conf ../bazel-bin/nic/hal/test/gtests/policer_test
	CAPRI_MOCK_MODE=1 WS_TOP=$(TOPDIR)/../../.. HAL_CONFIG_PATH=$(TOPDIR)/conf ../bazel-bin/nic/hal/test/gtests/queue_test
	CAPRI_MOCK_MODE=1 WS_TOP=$(TOPDIR)/../../.. HAL_CONFIG_PATH=$(TOPDIR)/conf ../bazel-bin/nic/hal/test/gtests/session_test
	#CAPRI_MOCK_MODE=1 WS_TOP=$(TOPDIR)/../../.. HAL_CONFIG_PATH=$(TOPDIR)/conf ../bazel-bin/nic/hal/test/gtests/tcpcb_test
	CAPRI_MOCK_MODE=1 WS_TOP=$(TOPDIR)/../../.. HAL_CONFIG_PATH=$(TOPDIR)/conf ../bazel-bin/nic/hal/test/gtests/tlscb_test
	CAPRI_MOCK_MODE=1 WS_TOP=$(TOPDIR)/../../.. HAL_CONFIG_PATH=$(TOPDIR)/conf ../bazel-bin/nic/hal/test/gtests/uplinkif_test
	CAPRI_MOCK_MODE=1 WS_TOP=$(TOPDIR)/../../.. HAL_CONFIG_PATH=$(TOPDIR)/conf ../bazel-bin/nic/hal/test/gtests/uplinkpc_test
	CAPRI_MOCK_MODE=1 WS_TOP=$(TOPDIR)/../../.. HAL_CONFIG_PATH=$(TOPDIR)/conf ../bazel-bin/nic/hal/test/gtests/internal_test
	CAPRI_MOCK_MODE=1 WS_TOP=$(TOPDIR)/../../.. HAL_CONFIG_PATH=$(TOPDIR)/conf ../bazel-bin/nic/hal/test/gtests/endpoint_test
	CAPRI_MOCK_MODE=1 WS_TOP=$(TOPDIR)/../../.. HAL_CONFIG_PATH=$(TOPDIR)/conf ../bazel-bin/nic/hal/test/gtests/tenant_test
	CAPRI_MOCK_MODE=1 WS_TOP=$(TOPDIR)/../../.. HAL_CONFIG_PATH=$(TOPDIR)/conf ../bazel-bin/nic/hal/test/gtests/l2seg_test
	# PI Utils gtests
	$(TOPDIR)/../bazel-bin/nic/utils/bitmap/bitmap_test
	$(TOPDIR)/../bazel-bin/nic/utils/host_mem/host_mem_test
	$(TOPDIR)/../bazel-bin/nic/utils/indexer/indexer_test
	$(TOPDIR)/../bazel-bin/nic/utils/list/list_test
	$(TOPDIR)/../bazel-bin/nic/utils/pt/pt_test
	$(TOPDIR)/../bazel-bin/nic/utils/shm/shmseg_test
	$(TOPDIR)/../bazel-bin/nic/utils/slab/slab_test
	$(TOPDIR)/../bazel-bin/nic/utils/twheel/twheel_test
	# PD Utils gtests
	LD_PRELOAD=$(TOPDIR)/../bazel-bin/nic/hal/pd/utils/p4pd_mock/libp4pd_mock.so HAL_CONFIG_PATH=$(TOPDIR)/conf $(TOPDIR)/../bazel-bin/nic/hal/pd/utils/acl_tcam/acl_tcam_test
	LD_PRELOAD=$(TOPDIR)/../bazel-bin/nic/hal/pd/utils/p4pd_mock/libp4pd_mock.so HAL_CONFIG_PATH=$(TOPDIR)/conf $(TOPDIR)/../bazel-bin/nic/hal/pd/utils/tcam/tcam_test
	LD_PRELOAD=$(TOPDIR)/../bazel-bin/nic/hal/pd/utils/p4pd_mock/libp4pd_mock.so HAL_CONFIG_PATH=$(TOPDIR)/conf $(TOPDIR)/../bazel-bin/nic/hal/pd/utils/directmap/directmap_test
	# Removed because of randomization
	#LD_PRELOAD=$(TOPDIR)/../bazel-bin/nic/hal/pd/utils/p4pd_mock/libp4pd_mock.so HAL_CONFIG_PATH=$(TOPDIR)/conf $(TOPDIR)/../bazel-bin/nic/hal/pd/utils/flow/flow_test
	LD_PRELOAD=$(TOPDIR)/../bazel-bin/nic/hal/pd/utils/p4pd_mock/libp4pd_mock.so HAL_CONFIG_PATH=$(TOPDIR)/conf $(TOPDIR)/../bazel-bin/nic/hal/pd/utils/hash/hash_test
	LD_PRELOAD=$(TOPDIR)/../bazel-bin/nic/hal/pd/utils/p4pd_mock/libp4pd_mock.so HAL_CONFIG_PATH=$(TOPDIR)/conf $(TOPDIR)/../bazel-bin/nic/hal/pd/utils/met/met_test

	
