.PHONY: default all pb test

default: all

all: pb build

client_test.pb.go: client_test.proto
	make -C ${GOPATH}/src/github.com/pensando/sw/nic/delphi/messanger/proto && \
	make -C ${GOPATH}/src/github.com/pensando/sw/nic/delphi/proto/delphi && \
	protoc --delphigo_out=Mnic/delphi/proto/delphi/delphi.proto=github.com/pensando/sw/nic/delphi/proto/delphi:. \
		./client_test.proto -I /usr/src/github.com/pensando/sw -I .

build:
	go build

test: client_test.pb.go
	go test -cover
