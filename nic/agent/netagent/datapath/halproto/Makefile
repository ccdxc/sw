GEN_DIR = ${CURDIR}
TOP_DIR = ../../../..
LD_LIBRARY_PATH=$(TOP_DIR)/hal/third-party/google/x86_64/lib/
HAL_PROTO_FILES = $(shell ls ${CURDIR}/../../../../proto/hal/ | grep .proto | grep -v module | tr " " "\n" | LC_ALL=C sort | tr "\n" " ")
HAL_PB_GO_FILES = $(patsubst %.proto,%.pb.go,$(HAL_PROTO_FILES))
HAL_MOCK_GO_FILES = $(patsubst %.pb.go,%_mock.go,$(HAL_PB_GO_FILES))
$(info ${HAL_PB_GO_FILES})
$(info ${HAL_MOCK_GO_FILES})
default: hal-proto
	for c in ${HAL_PROTO_FILES}; do printf "\n+++++++++++++++++ Generating Mocks For $${c} +++++++++++++++++\n"; mockgen -source=`basename $${c} .proto`.pb.go -destination=`basename $${c} .proto`_mock.go -package=halproto || exit 1; done
	# Run go imports on mock files
	goimports -w -l *mock.go

hal-proto: clean-proto
	# Compile proto definitions
	 LC_ALL=C LD_LIBRARY_PATH=$(LD_LIBRARY_PATH) $(TOP_DIR)/hal/third-party/google/x86_64/bin/protoc --proto_path=../../../../../ -I../../../../../nic/proto \
	-I../../../../../nic/proto/hal \
	-I../../../../../nic/delphi/proto/delphi \
	-I$(TOP_DIR)/hal/third-party/google/include \
	-I../../../../../vendor/github.com/gogo/protobuf/gogoproto \
	--gogofast_out=Mgogo.proto=github.com/gogo/protobuf/gogoproto,Mdelphi.proto=github.com/pensando/sw/nic/delphi/proto/delphi,Mgoogle/protobuf/any.proto=github.com/gogo/protobuf/types,plugins=grpc:. types.proto ${HAL_PROTO_FILES} events.proto

clean-proto:
	# Regenerate all .pb.go. This avoids conflicts due to file renames in nic/proto/hal
	rm -f ${GEN_DIR}/*.go

proto-sync: default
	# Will verify if the generated pb.go are committed
	@$(eval DIFFS=`git ls-files --exclude-standard --modified --directory ${CURDIR}`)
	@echo "Found the following uncommitted pb.go files, if any"
	@echo $(DIFFS)
	@test -z "$(DIFFS)"
