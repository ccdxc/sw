

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
		  hal/src \
		  hal/periodic \
		  hal/pd/iris/p4pd \
		  gen/iris/src \
		  gen/tcp_proxy_rxdma/src \
		  gen/tcp_proxy_txdma/src \
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
		  hal/test \
		  agents \
		  $(NULL)
SUBDIRS = $(LIBDIRS) $(PGMDIRS)

include $(TOPDIR)/make/Makefile.inc

# get a shell with the dependencies image loaded, with the host filesystem mounted.
shell: build-runtime-image
	docker run -it --privileged -v ${PWD}/../dol:/hack/saratk/dol -v ${PWD}:/hack/saratk/nic pensando/nic bash

# run `make gtest` with the dependencies image.
test: build-runtime-image
	docker run -it -v ${PWD}/../dol:/hack/saratk/dol -v ${PWD}:/hack/saratk/nic pensando/nic bash -c 'make gtest'

# run a build with the dependencies image.
build: build-runtime-image
	docker run -it -v ${PWD}/../dol:/hack/saratk/dol -v ${PWD}:/hack/saratk/nic pensando/nic sh -c 'make -j$$(grep -c processor /proc/cpuinfo)'

clean-docker: build-runtime-image
	docker run -it -v ${PWD}/../dol:/hack/saratk/dol -v ${PWD}:/hack/saratk/nic pensando/nic make clean

build-runtime-image: install_box
	if [ "x${NO_PULL}" = "x" ]; then docker pull srv1.pensando.io:5000/pensando/nic:dependencies; fi
	BOX_INCLUDE_ENV="NO_COPY" NO_COPY=1 box box.rb

install_box:
	@if [ ! -x /usr/local/bin/box ]; then echo "Installing box, sudo is required"; curl -sSL box-builder.sh | sudo bash; fi

# make a trial dependencies image. pass RELEASE=1 or run `make deps-release` to
# do a release build.
deps: install_box
	RELEASE=${RELEASE} BOX_INCLUDE_ENV="RELEASE" box -t 'srv1.pensando.io:5000/pensando/nic:dependencies' box-deps.rb

# make a release build of the dependencies image
deps-release:
	RELEASE=1 make deps

# make a shell based on the local build of the dependencies image, for testing
deps-test-shell:
	NO_PULL=1 make shell

# do a test build with a local build of the dependencies image.
deps-test-build:
	NO_PULL=1 make build
