.PHONY: all protobuf run_tests

all: run_test

protobuf:
	protoc --delphigo_out=Mnic/delphi/proto/delphi/delphi.proto=github.com/pensando/sw/nic/delphi/proto/delphi:. \
		/usr/src/github.com/pensando/sw/nic/upgrade_manager/proto/upgrade.proto -I /usr/src/github.com/pensando/sw -I .  -I /usr/src/github.com/pensando/sw/nic/upgrade_manager

run_test: protobuf
	go build

clean:
	rm -rf go nic
