.PHONY: all build clean

HAL_PROTO_FILES = $(shell ls ${CURDIR}/../../../../proto/hal/ | grep .proto | grep -v module | tr " " "\n" | LC_ALL=C sort | tr "\n" " ")

all: build

build: 
	protoc --delphigo_out=Mdelphi.proto=github.com/pensando/sw/nic/delphi/proto/delphi,Mgogo.proto=github.com/gogo/protobuf/gogoproto:./halproto ${HAL_PROTO_FILES} \
                -I ../../../../proto/hal/ -I /usr/src/github.com/pensando/sw -I . \
                -I ../../../../../vendor/github.com/gogo/protobuf/gogoproto/ \
                -I ../../../../delphi/proto/delphi/ 
	goimports -w ./halproto/*.go
	go build -o ${BLD_BIN_DIR}/delphidp_halproto  ./halproto/

clean:
	rm -f ./halproto/*.pb.go
	rm -f ./halproto/*.delphi.go
