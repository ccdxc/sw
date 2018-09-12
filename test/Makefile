default: all
TOPDIR = .

export TEST_AGENT_DIR      := ${TOPDIR}/utils/infra/agent
export TEST_AGENT_GRPC     := ${TEST_AGENT_DIR}/grpc
export TEST_AGENT_GRPC_API := ${TEST_AGENT_DIR}/grpc/api
export TEST_BIN_DIR	   := ${TOPDIR}/bin

testagent:
	 cd $(TEST_AGENT_GRPC_API) && protoc agent.proto --go_out=plugins=grpc:pb

e2e-turin:testagent
	mkdir -p $(TEST_BIN_DIR)
	cd $(GOPATH)/src/github.com/pensando/sw/test  && go build -ldflags="-s -w" -o $(TEST_BIN_DIR)/e2e-turin e2e/turin/main.go

all: e2e-turin

