SRC := $(shell ls *.proto)
DEST := $(SRC:%.proto=%.pb.go)
default: ${DEST}

%.pb.go: %.proto Makefile
	protoc --proto_path=./ \
          -I../../../../api \
          -I../../../../vendor/ \
          -I../../../../vendor/github.com/gogo/protobuf/gogoproto \
          -I../../../../vendor/github.com/pensando/grpc-gateway/third_party/googleapis \
          -I$(GOPATH)/src \
	--gofast_out=Mgogo.proto=github.com/gogo/protobuf/gogoproto,plugins=grpc:. $<
	goimports -local "github.com/pensando/sw" -w *.go

clean:
	rm -f *.pb.go
