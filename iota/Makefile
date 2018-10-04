default: all
TOPDIR                          := $(GOPATH)/src/github.com/pensando/sw/iota
export IOTA_CMD_DIR		:= ${TOPDIR}/svcs/cmd
export IOTA_AGENT_DIR           := ${IOTA_CMD_DIR}/iota-agent
export IOTA_SERVER_DIR		:= ${IOTA_CMD_DIR}/iota-server

export IOTA_BIN_DIR		:= ${TOPDIR}/bin
export IOTA_SERVER_BIN_DIR	:= ${IOTA_BIN_DIR}/server
export IOTA_AGENT_BIN_DIR	:= ${IOTA_BIN_DIR}/agent

iota-server:iota-proto
	mkdir -p $(IOTA_SERVER_BIN_DIR)
	go build -ldflags="-s -w" -o $(IOTA_SERVER_BIN_DIR)/iota_server github.com/pensando/sw/iota/svcs/cmd/iota-server

iota-proto:
	mkdir -p $(TOPDIR)/protos/pygen
	mkdir -p $(TOPDIR)/protos/gogen
	make  -C $(TOPDIR)/protos

iota-agent:iota-proto
	mkdir -p $(IOTA_AGENT_BIN_DIR)
	go build -ldflags="-s -w" -o $(IOTA_AGENT_BIN_DIR)/iota_agent github.com/pensando/sw/iota/svcs/cmd/iota-agent

iota: iota-proto iota-agent iota-server

.PHONY: test
test:
	cd ${TOPDIR}
	go test -v ./...
clean:
	rm -rf ${IOTA_BIN_DIR}

all: iota
