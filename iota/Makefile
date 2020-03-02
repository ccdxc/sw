default: all
TOPDIR                          		:= $(GOPATH)/src/github.com/pensando/sw/iota
NICDIR                          		:= $(GOPATH)/src/github.com/pensando/sw/nic
export IOTA_CMD_DIR             		:= ${TOPDIR}/svcs/cmd
export IOTA_AGENT_DIR           		:= ${IOTA_CMD_DIR}/iota-agent
export IOTA_SERVER_DIR          		:= ${IOTA_CMD_DIR}/iota-server
export IOTA_BIN_DIR             		:= ${TOPDIR}/bin
export IOTA_SERVER_BIN_DIR      		:= ${IOTA_BIN_DIR}/server
export IOTA_AGENT_BIN_LINUX_DIR 		:= ${IOTA_BIN_DIR}/agent/linux
export IOTA_AGENT_BIN_FREEBSD_DIR 		:= ${IOTA_BIN_DIR}/agent/freebsd
export IOTA_AGENT_TEST_BIN_LINUX_DIR  	:= ${IOTA_BIN_DIR}/agent/test/linux
export IOTA_AGENT_TEST_BIN_FREEBSD_DIR  := ${IOTA_BIN_DIR}/agent/test/freebsd
export IOTA_PENCTL_DIR                  := ${IOTA_BIN_DIR}/penctl
SRCS := $(shell find ./ -name *.go)


bin/server/iota_server: ${SRCS}
	mkdir -p $(IOTA_SERVER_BIN_DIR)
	go build -ldflags="-s -w" -o $(IOTA_SERVER_BIN_DIR)/iota_server github.com/pensando/sw/iota/svcs/cmd/iota-server

bin/tools/fuz: ${SRCS}
	mkdir -p $(IOTA_BIN_DIR)
	go build -o $(IOTA_BIN_DIR)/fuz github.com/pensando/sw/iota/tools/fuz

bin/esx-setup: ${SRCS}
	go build -ldflags="-s -w" -o $(IOTA_BIN_DIR)/iota_esx_setup github.com/pensando/sw/iota/scripts/esx

iota-proto:
	make  -C $(TOPDIR)/protos

bin/agent/iota_agent: ${SRCS}
	mkdir -p $(IOTA_AGENT_BIN_LINUX_DIR)
	mkdir -p $(IOTA_AGENT_TEST_BIN_LINUX_DIR)
	go build -ldflags="-s -w" -o $(IOTA_AGENT_BIN_LINUX_DIR)/iota_agent github.com/pensando/sw/iota/svcs/cmd/iota-agent
	go build -ldflags="-s -w" -o $(IOTA_AGENT_TEST_BIN_LINUX_DIR)/iota-agent-test github.com/pensando/sw/iota/svcs/agent/test

bin/agent/iota_agent.freebsd: ${SRCS}
	mkdir -p $(IOTA_AGENT_BIN_FREEBSD_DIR)
	mkdir -p $(IOTA_AGENT_TEST_BIN_FREEBSD_DIR)
	GOOS=freebsd go build -ldflags="-s -w" -o $(IOTA_AGENT_BIN_FREEBSD_DIR)/iota_agent github.com/pensando/sw/iota/svcs/cmd/iota-agent
	GOOS=freebsd go build -ldflags="-s -w" -o $(IOTA_AGENT_TEST_BIN_FREEBSD_DIR)/iota-agent-test github.com/pensando/sw/iota/svcs/agent/test


bin/iota-cmd:
	go build -ldflags="-s -w" -o $(IOTA_BIN_DIR)/iota_cmd github.com/pensando/sw/iota/test/venice/iotacmd

bin/penctl:
	#Hack for now, once cleaned up, this dep should go away
	make -C ../nic/upgrade_manager/export/upggosdk
	mkdir -p $(IOTA_PENCTL_DIR)
	GOOS=freebsd go build -ldflags="-s -w" -o $(IOTA_PENCTL_DIR)/penctl.freebsd github.com/pensando/sw/penctl 
	GOOS=linux go build -ldflags="-s -w" -o $(IOTA_PENCTL_DIR)/penctl.linux github.com/pensando/sw/penctl 

iota-test:
	go test -c ./svcs/agent/ -o bin/agent.test

iota-test-bins:
	go test -c github.com/pensando/sw/iota/test/venice/testsuites/equinix -o bin/equinix.test


iota: iota-proto bin/penctl bin/server/iota_server bin/agent/iota_agent bin/agent/iota_agent.freebsd bin/esx-setup bin/tools/fuz bin/iota-cmd iota-test iota-test-bins

iota-scale-build: iota heimdall

iota-tarball: iota
	tar -zcf iota.tgz bin images protos && cp iota.tgz ../ 

heimdall:
	make -C $(NICDIR) heimdall.gobin
	cp $(NICDIR)/build/x86_64/iris/bin/heimdall $(IOTA_BIN_DIR)/

.PHONY: test
test:
	cd ${TOPDIR}
	go test -v ./...

clean:
	make -C $(TOPDIR)/protos clean
	rm -rf ${IOTA_BIN_DIR}

.PHONY: images
images:
	$(MAKE) -C workload-images deps-release
	echo "Please do a docker push manually."

all: iota


REGISTRY = registry.test.pensando.io:5000
NIC_CONTAINER_VERSION:=1.35
CUR_DIR:=$(shell pwd)
SW_DIR:=$(shell dirname ${CUR_DIR})
CUR_USER:=$(shell whoami)
CUR_TIME:=$(shell date +%Y-%m-%d_%H.%M.%S)
CONTAINER_NAME:=${CUR_USER}_${CUR_TIME}
docker/install_box:
	@if [ ! -x /usr/local/bin/box ]; then echo "Installing box, sudo is required"; curl -sSL box-builder.sh | sudo bash; fi
	if [ "x${NO_PULL}" = "x" ]; then docker pull $(REGISTRY)/pensando/nic:${NIC_CONTAINER_VERSION}; fi

docker/build-runtime-image: docker/install_box
	cd .. && BOX_INCLUDE_ENV="NO_COPY USER USER_UID USER_GID GROUP_NAME" NO_COPY=1 USER_UID=$$(id -u) USER_GID=$$(id -g) GROUP_NAME=$$(id -gn) box -t pensando/nic nic/box.rb

docker/build-iota-runtime-image: docker/install_box
	cd .. && BOX_INCLUDE_ENV="NO_COPY USER USER_UID USER_GID GROUP_NAME" NO_COPY=1 USER_UID=$$(id -u) USER_GID=$$(id -g) GROUP_NAME=$$(id -gn) box -t pensando/nic iota/box.rb

docker/run: docker/build-runtime-image
	docker run --rm --sysctl net.ipv6.conf.all.disable_ipv6=0 --privileged --name ${CONTAINER_NAME} -v $(SW_DIR):/sw -v /vol/builds:/vol/builds -w /sw/iota pensando/nic su -l $(CUR_USER) -c 'set -o pipefail && cd ../iota && make && ./iota.py --testbed ${HARDWARE_TESTBED_COPIED} ${EXTRA_ARGS}  2>&1 | tee run.log'

docker/run-iota: docker/build-iota-runtime-image
	docker run --rm --sysctl net.ipv6.conf.all.disable_ipv6=0 --privileged --name ${CONTAINER_NAME} -v $(SW_DIR):/sw -v /vol/builds:/vol/builds -w /sw/iota pensando/nic su -l $(CUR_USER) -c 'set -o pipefail && cd ../iota && ./iota.py --testbed ${HARDWARE_TESTBED_COPIED} ${EXTRA_ARGS}  2>&1 | tee run.log'

docker/run-e2e-iota: docker/build-iota-runtime-image
	docker run --rm --sysctl net.ipv6.conf.all.disable_ipv6=0 --privileged --name ${CONTAINER_NAME} -v $(SW_DIR):/sw -v /vol/builds:/vol/builds -w /sw/iota pensando/nic su -l $(CUR_USER) -c 'set -o pipefail && cd ../iota && test/venice/start_iota.sh &&  ${IOTA_RUN_CMD} -testbed ${HARDWARE_TESTBED_COPIED}  2>&1 | tee run.log'

docker/build-iota: docker/build-runtime-image
	docker run --rm --sysctl net.ipv6.conf.all.disable_ipv6=0 --privileged --name ${CONTAINER_NAME} -v $(SW_DIR):/sw -v /vol/builds:/vol/builds -w /sw/iota pensando/nic su -l $(CUR_USER) -c 'set -o pipefail && cd ../iota && make'

