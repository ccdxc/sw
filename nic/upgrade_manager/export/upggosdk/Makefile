.PHONY: all protobuf run_tests

all: build

pb:
	protoc --delphigo_out=Mdelphi.proto=github.com/pensando/sw/nic/delphi/proto/delphi,Mpenctl.proto=github.com/pensando/sw/venice/utils/apigen/annotations:. \
		${GOPATH}/src/github.com/pensando/sw/nic/upgrade_manager/upgrade/upgrade.proto -I${GOPATH}/src/github.com/pensando/sw -I${GOPATH}/src/github.com/pensando/sw/nic/upgrade_manager -I${GOPATH}/src/github.com/pensando/sw/nic/delphi/proto/delphi                 -I${GOPATH}/src \
                -I${GOPATH}/src/github.com/pensando/sw/venice/utils/apigen/annotations/ \
                -I${GOPATH}/src/github.com/pensando/sw/vendor/github.com/pensando/grpc-gateway/third_party \
                -I${GOPATH}/src/github.com/pensando/sw/vendor/github.com/gogo/protobuf/protobuf \
                -I${GOPATH}/src/github.com/pensando/sw/vendor/github.com/pensando/grpc-gateway/third_party/googleapis \
                -I${GOPATH}/src/github.com/pensando/sw/vendor/github.com/gogo/protobuf/gogoproto \
                -I${GOPATH}/src/github.com/pensando/sw/nic/delphi/proto/delphi/ \

build: pb

gen-clean: ;
