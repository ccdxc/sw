FIX_PROTO = sed -i.bak_ 's/github.com\/golang\/protobuf\/proto/github.com\/gogo\/protobuf\/proto/'
FIX_VER = sed -i.bak_ 's/proto.ProtoPackageIsVersion/proto.GoGoProtoPackageIsVersion/'
DEL_BAK = rm *.bak_
SCHEMA_PROTOS = meta.proto common.proto export.proto

TO_GEN = bulkedit labels fields
all: generated/swagger/rice-box.go

generated/swagger/rice-box.go: meta.proto common.proto $(shell ls protos/*.proto)
	$(MAKE) bld_proto
	$(MAKE) fix
	$(MAKE) genschema
	$(MAKE) genapi
	$(MAKE) -C mock

fix:
	@${FIX_VER} *pb.go
	@${FIX_PROTO} *pb.go
	@${DEL_BAK}

bld_proto:
	@protoc -I. -I../vendor -I${GOPATH}/src \
		-I${GOPATH}/src/github.com/pensando/sw/vendor/github.com/pensando/grpc-gateway/third_party/googleapis \
		-I${GOPATH}/src/github.com/pensando/sw/vendor/github.com/gogo/protobuf/protobuf \
		--pensando_out=custom_enumstr=true,Mgoogle/protobuf/timestamp.proto=github.com/gogo/protobuf/types,Mgoogle/protobuf/any.proto=github.com/gogo/protobuf/types,Mgoogle/protobuf/descriptor.proto=github.com/gogo/protobuf/protoc-gen-gogo/descriptor:. *.proto
	@goimports -local "github.com/pensando/sw" -l -w *.pb.go
	@for c in ${TO_GEN}; do printf "\n+++++++++++++++++ Generating $${c} +++++++++++++++++\n"; make -C $${c} || exit 1; done

genschema:
	${GOPATH}/src/github.com/pensando/sw/tools/scripts/genschema.sh ${SCHEMA_PROTOS}

genapi:
	${GOPATH}/src/github.com/pensando/sw/tools/scripts/genapi.sh protos

clean:
	@rm *.pb.go *_schema.go
	$(MAKE) -C mock clean
	@rm -rf generated/*

gen-clean: clean
