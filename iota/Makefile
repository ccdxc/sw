default: all
TOPDIR                          := $(GOPATH)/src/github.com/pensando/sw/iota
export IOTA_CMD_DIR             := ${TOPDIR}/svcs/cmd
export IOTA_AGENT_DIR           := ${IOTA_CMD_DIR}/iota-agent
export IOTA_SERVER_DIR          := ${IOTA_CMD_DIR}/iota-server
export IOTA_BIN_DIR             := ${TOPDIR}/bin
export IOTA_SERVER_BIN_DIR      := ${IOTA_BIN_DIR}/server
export IOTA_AGENT_BIN_DIR       := ${IOTA_BIN_DIR}/agent
export IOTA_AGENT_TEST_BIN_DIR  := ${IOTA_BIN_DIR}/agent/test
SRCS := $(shell find ./ -name *.go)

bin/server/iota_server: ${SRCS}
	mkdir -p $(IOTA_SERVER_BIN_DIR)
	go build -ldflags="-s -w" -o $(IOTA_SERVER_BIN_DIR)/iota_server github.com/pensando/sw/iota/svcs/cmd/iota-server

iota-proto:
	make  -C $(TOPDIR)/protos

bin/agent/iota_agent: ${SRCS}
	mkdir -p $(IOTA_AGENT_BIN_DIR)
	mkdir -p $(IOTA_AGENT_TEST_BIN_DIR)
	go build -ldflags="-s -w" -o $(IOTA_AGENT_BIN_DIR)/iota_agent github.com/pensando/sw/iota/svcs/cmd/iota-agent
	go build -ldflags="-s -w" -o $(IOTA_AGENT_TEST_BIN_DIR)/iota-agent-test github.com/pensando/sw/iota/svcs/agent/test

iota: iota-proto bin/server/iota_server bin/agent/iota_agent

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
