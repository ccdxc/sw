default: all
TOPDIR                          		:= $(GOPATH)/src/github.com/pensando/sw/iota
export IOTA_CMD_DIR             		:= ${TOPDIR}/svcs/cmd
export IOTA_AGENT_DIR           		:= ${IOTA_CMD_DIR}/iota-agent
export IOTA_SERVER_DIR          		:= ${IOTA_CMD_DIR}/iota-server
export IOTA_BIN_DIR             		:= ${TOPDIR}/bin
export IOTA_SERVER_BIN_DIR      		:= ${IOTA_BIN_DIR}/server
export IOTA_AGENT_BIN_LINUX_DIR 		:= ${IOTA_BIN_DIR}/agent/linux
export IOTA_AGENT_BIN_FREEBSD_DIR 		:= ${IOTA_BIN_DIR}/agent/freebsd
export IOTA_AGENT_TEST_BIN_LINUX_DIR  	:= ${IOTA_BIN_DIR}/agent/test/linux
export IOTA_AGENT_TEST_BIN_FREEBSD_DIR  := ${IOTA_BIN_DIR}/agent/test/freebsd
SRCS := $(shell find ./ -name *.go)


bin/server/iota_server: ${SRCS}
	mkdir -p $(IOTA_SERVER_BIN_DIR)
	go build -ldflags="-s -w" -o $(IOTA_SERVER_BIN_DIR)/iota_server github.com/pensando/sw/iota/svcs/cmd/iota-server

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

iota: iota-proto bin/server/iota_server bin/agent/iota_agent bin/agent/iota_agent.freebsd bin/esx-setup

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
