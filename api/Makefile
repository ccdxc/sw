FIX_PROTO = sed -i.bak_ 's/github.com\/golang\/protobuf\/proto/github.com\/gogo\/protobuf\/proto/'
FIX_VER = sed -i.bak_ 's/proto.ProtoPackageIsVersion/proto.GoGoProtoPackageIsVersion/'
DEL_BAK = rm *.bak_

all:
	$(MAKE) bld_proto
	$(MAKE) fix
	$(MAKE) genapi

fix:
	@${FIX_VER} *pb.go
	@${FIX_PROTO} *pb.go
	@${DEL_BAK}

bld_proto:
	@protoc -I/usr/local/include -I. -I../vendor \
		-I${GOPATH}/src/github.com/pensando/sw/vendor/github.com/pensando/grpc-gateway/third_party/googleapis \
		--gofast_out=Mgoogle/protobuf/timestamp.proto=github.com/gogo/protobuf/types,Mgoogle/protobuf/descriptor.proto=github.com/gogo/protobuf/protoc-gen-gogo/descriptor:. *.proto
	@gofmt -s -w *.pb.go

genapi:
	${GOPATH}/src/github.com/pensando/sw/tools/scripts/genapi.sh protos	

clean:
	@rm -rf generated/*

