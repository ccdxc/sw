FIX_PROTO = sed -i.bak_ 's/github.com\/golang\/protobuf\/proto/github.com\/gogo\/protobuf\/proto/'
FIX_VER = sed -i.bak_ 's/proto.ProtoPackageIsVersion/proto.GoGoProtoPackageIsVersion/'
DEL_BAK = rm *.bak_

all:
	$(MAKE) bld_proto
	$(MAKE) fix

fix:
	@${FIX_VER} *pb.go
	@${FIX_PROTO} *pb.go
	@${DEL_BAK}

bld_proto:
	@protoc -I/usr/local/include -I. \
        -I${GOPATH}/src \
        -I${GOPATH}/src/github.com/pensando/sw/api/protos \
        -I${GOPATH}/src/github.com/pensando/sw/vendor/github.com/pensando/grpc-gateway/third_party/googleapis \
        -I${GOPATH}/src/github.com/pensando/sw/vendor/github.com/pensando/grpc-gateway/third_party \
        -I${GOPATH}/src/github.com/pensando/sw/vendor/github.com/gogo/protobuf/protobuf \
        -I${GOPATH}/src/github.com/pensando/sw/vendor \
        --pensando_out=Mgoogle/protobuf/any.proto=github.com/gogo/protobuf/types,plugins=grpc,logtostderr=false,plugins=grpc:. *.proto
	@goimports -local "github.com/pensando/sw" -l -w *.go