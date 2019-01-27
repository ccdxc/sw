GEN_DIR = ${CURDIR}
TOP_DIR = ../../../..
LD_LIBRARY_PATH=$(TOP_DIR)/hal/third-party/google/x86_64/lib/
HAL_PROTO_FILES = device.proto
HAL_PB_GO_FILES = $(patsubst %.proto,%.pb.go,$(HAL_PROTO_FILES))
$(info ${HAL_PB_GO_FILES})

hal-proto: clean-proto
	# Compile proto definitions
	 LC_ALL=C LD_LIBRARY_PATH=$(LD_LIBRARY_PATH) $(TOP_DIR)/hal/third-party/google/x86_64/bin/protoc --proto_path=../../../../../ -I../../../../../nic/proto \
	-I../../../../../nic/proto/hal \
	-I../../../../../nic/delphi/proto/delphi \
	-I$(TOP_DIR)/hal/third-party/google/include \
	-I../../../../../vendor/github.com/gogo/protobuf/gogoproto \
	--gogofast_out=Mgogo.proto=github.com/gogo/protobuf/gogoproto,Mdelphi.proto=github.com/pensando/sw/nic/delphi/proto/delphi,Mgoogle/protobuf/any.proto=github.com/gogo/protobuf/types,plugins=grpc:. ${HAL_PROTO_FILES}

clean-proto:
	# Regenerate all .pb.go. This avoids conflicts due to file renames in nic/proto/hal
	rm -f ${GEN_DIR}/*.go
