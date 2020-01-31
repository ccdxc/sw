GEN_DIR = ${CURDIR}
TOP_DIR = ../../../..
LD_LIBRARY_PATH=$(TOP_DIR)/hal/third-party/google/x86_64/lib/
HAL_PROTO_FILES = endpoint.proto eplearn.proto event.proto events.proto fwlog.proto interface.proto l2segment.proto multicast.proto nwsec.proto port.proto qos.proto system.proto telemetry.proto vrf.proto
HAL_PB_GO_FILES = $(patsubst %.proto,%.pb.go,$(HAL_PROTO_FILES))
$(info ${HAL_PB_GO_FILES})

default: clean-proto
	# Compile proto definitions
	 LC_ALL=C LD_LIBRARY_PATH=$(LD_LIBRARY_PATH) protoc \
    --proto_path=../../../../../ -I../../../../../nic/proto \
	-I../../../../../nic/proto/hal \
	-I../../../../../nic/delphi/proto/delphi \
	-I$(TOP_DIR)/hal/third-party/google/include \
	-I../../../../../vendor/github.com/gogo/protobuf/gogoproto \
	--gogofast_out=Mgogo.proto=github.com/gogo/protobuf/gogoproto,Mdelphi.proto=github.com/pensando/sw/nic/delphi/proto/delphi,Mgoogle/protobuf/any.proto=github.com/gogo/protobuf/types,plugins=grpc:. types.proto kh.proto ${HAL_PROTO_FILES}

clean-proto:
	# Regenerate all .pb.go. This avoids conflicts due to file renames in nic/proto/hal
	rm -f ${GEN_DIR}/*.go

proto-sync: default
	# Will verify if the generated pb.go are committed
	@$(eval DIFFS=`git ls-files --exclude-standard --modified --directory ${CURDIR}`)
	@echo "Found the following uncommitted pb.go files, if any"
	@echo $(DIFFS)
	@test -z "$(DIFFS)"
