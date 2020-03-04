GEN_DIR = ${CURDIR}
TOP_DIR = ../../../../..
LD_LIBRARY_PATH=$(TOP_DIR)/nic/hal/third-party/google/x86_64/lib/
PROTO_DIR = ${TOP_DIR}/nic/apollo/agent/protos/
HAL_PROTO_FILES = types.proto vpc.proto device.proto policy.proto subnet.proto \
                  dhcp.proto interface.proto port.proto event.proto oper.proto \
                  cp_route.proto bgp.proto evpn.proto
PROTOSUBST = Mgogo.proto=github.com/gogo/protobuf/gogoproto,Mmeta/meta.proto=github.com/pensando/sw/nic/apollo/agent/gen/pds/meta/pds,Mtypes.proto=github.com/pensando/sw/nic/apollo/agent/gen/pds,Mcommon.proto=ithub.com/pensando/sw/nic/metaswitch/gen/agent,Mpdsa.proto=github.com/pensando/sw/venice/utils/apigen/annotations

default: clean-proto
	# Compile proto definitions
	LC_ALL=C LD_LIBRARY_PATH=$(LD_LIBRARY_PATH) protoc \
	--proto_path=${PROTO_DIR} -I${PROTO_DIR} \
	-I${TOP_DIR}/nic/hal/third-party/google/include \
	-I${TOP_DIR}/vendor/github.com/gogo/protobuf/gogoproto \
	--gogofast_out=Mgogo.proto=github.com/gogo/protobuf/gogoproto,Mmeta/meta.proto=github.com/pensando/sw/nic/apollo/agent/gen/pds/meta/pds,plugins=grpc:. ${HAL_PROTO_FILES}
	LC_ALL=C LD_LIBRARY_PATH=$(LD_LIBRARY_PATH) protoc \
	--proto_path=${PROTO_DIR}/meta -I${PROTO_DIR}/meta \
	-I${TOP_DIR}/nic/hal/third-party/google/include \
	-I${TOP_DIR}/vendor/github.com/gogo/protobuf/gogoproto \
	--gogofast_out=Mgogo.proto=github.com/gogo/protobuf/gogoproto,Mgoogle/protobuf/timestamp.proto=github.com/gogo/protobuf/types,plugins=grpc:. meta.proto

create-gen-dir:
	mkdir -p ${CURDIR}/metaswitch
	mkdir -p ${TOP_DIR}/nic/metaswitch/gen/agent

clean-proto:
	# Regenerate all .pb.go. This avoids conflicts due to file renames in nic/proto/hal
	rm -f ${GEN_DIR}/*.go
	rm -f ${GEN_DIR}/meta/pds/*.go
	rm -f ${GEN_DIR}/metaswitch/*.go

gen-clean: clean-proto

proto-sync: default
	# Will verify if the generated pb.go are committed
	@$(eval DIFFS=`git ls-files --exclude-standard --modified --directory ${CURDIR}`)
	@echo "Found the following uncommitted pb.go files, if any"
	@echo $(DIFFS)
	@test -z "$(DIFFS)"
