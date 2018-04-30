CUR_DIR:=$(shell pwd)
STORAGE_DIR:=$(shell dirname ${CUR_DIR})
SW_DIR:=$(shell dirname ${STORAGE_DIR})
CUR_USER:=$(shell whoami)
CUR_TIME:=$(shell date +%Y-%m-%d_%H.%M.%S)
CONTAINER_NAME:=${CUR_USER}_${CUR_TIME}

# get a shell with the dependencies image loaded, with the host filesystem mounted.
shell: build-runtime-image
	mkdir -p ${SW_DIR}/bazel-cache;
	docker run -it --rm --sysctl net.ipv6.conf.all.disable_ipv6=0 --privileged --name ${CONTAINER_NAME} -v $(SW_DIR):/sw -v $(SW_DIR)/bazel-cache:/root/.cache -w /sw/storage/offload pensando/offload bash

REGISTRY = registry.test.pensando.io:5000

build-runtime-image: install_box
	if [ "x${NO_PULL}" = "x" ]; then docker pull $(REGISTRY)/pensando/nic:1.22; fi
	cd ../.. && BOX_INCLUDE_ENV="NO_COPY" NO_COPY=1 box -t pensando/offload storage/offload/box.rb

install_box:
	@if [ ! -x /usr/local/bin/box ]; then echo "Installing box, sudo is required"; curl -sSL box-builder.sh | sudo bash; fi
