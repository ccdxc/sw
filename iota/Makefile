default: all
TOPDIR                          := $(GOPATH)/src/github.com/pensando/sw/iota
export IOTA_AGENT_DIR           := ${TOPDIR}/svcs/agent
export IOTA_SVCS_DIR            := ${TOPDIR}/svcs
export IOTA_AGENT_BIN_DIR       := ${TOPDIR}/bin/iota-agent
export IOTA_SVC_BIN_DIR         := ${TOPDIR}/bin

iota-svc:iota-agent iota-proto
	mkdir -p $(IOTA_SVC_BIN_DIR)
	cd $(IOTA_SVCS_DIR)/cmd/iota  && go build -ldflags="-s -w" -o $(IOTA_SVC_BIN_DIR)/iota-svc main.go

iota-proto:
	mkdir -p $(TOPDIR)/protos/pygen
	mkdir -p $(TOPDIR)/protos/gogen
	make  -C $(TOPDIR)/protos

iota-agent:iota-proto
	mkdir -p $(IOTA_AGENT_BIN_DIR)
	cd $(IOTA_AGENT_DIR)/stub  &&  GOOS=linux GOARCH=amd64 go build -ldflags="-s -w" -o $(IOTA_AGENT_BIN_DIR)/iota-agent-stub .

iota:iota-svc


all: iota
