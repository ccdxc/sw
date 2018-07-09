.PHONY: all protobuf run_tests

all: build

pb:
	protoc --delphigo_out=Mnic/delphi/proto/delphi/delphi.proto=github.com/pensando/sw/nic/delphi/proto/delphi:. \
		/usr/src/github.com/pensando/sw/nic/upgrade_manager/upgrade/upgrade.proto -I /usr/src/github.com/pensando/sw -I /usr/src/github.com/pensando/sw/nic/upgrade_manager

build: pb
	go build
