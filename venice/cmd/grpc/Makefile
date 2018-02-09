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
	@protoc -I/usr/local/include -I. -I../../../vendor \
                -I${GOPATH}/src \
                -I../../../api \
                -I../../../api/protos \
		-I../../../vendor/github.com/pensando/grpc-gateway/third_party \
                -I../../../vendor/github.com/pensando/grpc-gateway/third_party/googleapis \
		--gofast_out=plugins=grpc,Mgithub.com/pensando/sw/api/protos/cmd.proto=github.com/pensando/sw/api/generated/cmd,Mgoogle/protobuf/descriptor.proto=github.com/gogo/protobuf/protoc-gen-gogo/descriptor:. cluster.proto
	@goimports -local "github.com/pensando/sw" -l -w *pb.go
