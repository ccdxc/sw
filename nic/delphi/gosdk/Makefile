.PHONY: default all pb test

default: all

all: pb build

pb:
	make -C ${GOPATH}/src/github.com/pensando/sw/nic/delphi/messanger/proto && \
	make -C ${GOPATH}/src/github.com/pensando/sw/nic/delphi/proto/delphi

build:
	go build
