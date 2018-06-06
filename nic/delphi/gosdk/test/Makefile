.PHONY: all protobuf run_tests

all: run_test

protobuf:
	protoc --delphigo_out=Mnic/delphi/proto/delphi/delphi.proto=github.com/pensando/sw/nic/delphi/proto/delphi:. \
		./interface.proto -I /usr/src/github.com/pensando/sw -I .

run_test: protobuf
	go test
