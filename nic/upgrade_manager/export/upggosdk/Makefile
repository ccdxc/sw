.PHONY: all protobuf run_tests

all: build

pb:
	protoc --delphigo_out=Mdelphi.proto=github.com/pensando/sw/nic/delphi/proto/delphi:. \
		/usr/src/github.com/pensando/sw/nic/upgrade_manager/upgrade/upgrade.proto -I /usr/src/github.com/pensando/sw -I /usr/src/github.com/pensando/sw/nic/upgrade_manager -I /usr/src/github.com/pensando/sw/nic/delphi/proto/delphi

build: pb
	go build
