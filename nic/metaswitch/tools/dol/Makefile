#################################################
################### DOCKER ######################
#################################################
CUR_DIR:=$(shell pwd)
SW_DIR:=$(shell dirname ${CUR_DIR})
CUR_USER:=$(shell whoami)
CUR_TIME:=$(shell date +%Y-%m-%d_%H.%M.%S)
CONTAINER_NAME:=${CUR_USER}_${CUR_TIME}
NIC_CONTAINER_VERSION:=1.41

REGISTRY = registry.test.pensando.io:5000

docker/build-runtime-ctr: docker/install_box
	if [ "x${NO_PULL}" = "x" ]; then docker pull $(REGISTRY)/pensando/nic:${NIC_CONTAINER_VERSION}; fi
	BOX_INCLUDE_ENV="NO_COPY USER USER_UID USER_GID GROUP_NAME" NO_COPY=1 USER_UID=$$(id -u) USER_GID=$$(id -g) GROUP_NAME=$$(id -gn) box -t pensando/nic box.rb

docker/install_box:
	@if [ ! -x /usr/local/bin/box ]; then echo "Installing box, sudo is required"; curl -sSL box-builder.sh | sudo bash; fi
