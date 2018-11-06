.PHONY: all build clean

all: build

interface.delphi.go: interface.proto
	protoc --delphigo_out=Mdelphi.proto=github.com/pensando/sw/nic/delphi/proto/delphi,Mgogo.proto=github.com/gogo/protobuf/gogoproto:. \
		interface.proto -I /usr/src/github.com/pensando/sw -I . -I ../../../../../../vendor/github.com/gogo/protobuf/gogoproto/ \
		-I ../../../../../delphi/proto/delphi/

build: interface.delphi.go
	go build

clean:
	rm -f example *.pb.go
	rm -f example *.delphi.go
