

TOPDIR = .
LIBDIRS = \
		  utils \
		  hal/pd/capri/csr \
		  proto gen/protobuf \
		  p4 \
		  capsim \
		  asm/nw \
		  model_sim/build \
		  fte \
		  hal/svc \
		  hal/src \
		  hal/periodic \
		  hal/pd/iris/p4pd \
		  gen/iris/src \
		  gen/tcp_proxy_rxdma/src \
		  gen/tcp_proxy_txdma/src \
		  hal/pd/common \
		  hal/pd/iris \
		  hal/pd/capri \
		  hal/pd/utils/directmap\
		  hal/pd/utils/flow \
		  hal/pd/utils/hash \
		  hal/pd/utils/met \
		  hal/pd/utils/tcam \
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
		  hal/pd/utils/directmap/test \
		  hal/pd/utils/flow/test \
		  hal/pd/utils/hash/test \
		  hal/pd/utils/met/test \
		  hal/pd/utils/tcam/test \
		  hal/test \
		  agents \
		  $(NULL)
SUBDIRS = $(LIBDIRS) $(PGMDIRS)

include $(TOPDIR)/make/Makefile.inc

shell: build-image
	docker run -it --privileged -v ${PWD}/../dol:/hack/saratk/dol -v ${PWD}:/hack/saratk/nic pensando/nic bash

test: build-image
	docker run -it -v ${PWD}/../dol:/hack/saratk/dol -v ${PWD}:/hack/saratk/nic pensando/nic bash -c 'make gtest'

build: build-image
	docker run -it -v ${PWD}/../dol:/hack/saratk/dol -v ${PWD}:/hack/saratk/nic pensando/nic sh -c 'make -j$$(grep -c processor /proc/cpuinfo)'

clean-docker: build-image
	docker run -it -v ${PWD}/../dol:/hack/saratk/dol -v ${PWD}:/hack/saratk/nic pensando/nic make clean

build-image: install_box
	docker pull srv1.pensando.io:5000/pensando/nic:dependencies
	BOX_INCLUDE_ENV="NO_COPY" NO_COPY=1 box box.rb

install_box:
	@if [ ! -x /usr/local/bin/box ]; then echo "Installing box, sudo is required"; curl -sSL box-builder.sh | sudo bash; fi

deps: install_box
	RELEASE=${RELEASE} box box-deps.rb
