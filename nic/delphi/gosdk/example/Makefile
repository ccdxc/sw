.PHONY: all build clean

all: build

upgrade.pb.go: upgrade.proto
	protoc --delphigo_out=Mnic/delphi/proto/delphi/delphi.proto=github.com/pensando/sw/nic/delphi/proto/delphi:. \
		./upgrade.proto -I /usr/src/github.com/pensando/sw -I . -I ${GOPATH}/src/github.com/pensando/sw

build: upgrade.pb.go
	go build

clean:
	rm -f example upgrade.pb.go
