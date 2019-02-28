GEN_DIR = ${CURDIR}
TOP_DIR = ../../../..
LD_LIBRARY_PATH=$(TOP_DIR)/nic/hal/third-party/google/x86_64/lib/
HAL_PROTO_FILES = $(shell ls ${CURDIR}/ | grep .proto | grep -v module | tr " " "\n" | LC_ALL=C sort | tr "\n" " ")
HAL_PB_GO_FILES = $(patsubst %.proto,%.pb.go,$(HAL_PROTO_FILES))
$(info ${HAL_PROTO_FILES})
$(info ${HAL_PB_GO_FILES})

default: tpc-proto
	goimports -w -l *.go

tpc-proto: clean-proto
    # compile proto definitions
	LC_ALL=C LD_LIBRARY_PATH=$(LD_LIBRARY_PATH) $(TOP_DIR)/nic/hal/third-party/google/x86_64/bin/protoc --proto_path=${CURDIR} -I${CURDIR} \
    -I${TOP_DIR} -I$(TOP_DIR)/vendor -I${CURDIR}/api -I$(TOP_DIR)/nic/hal/third-party/google/include \
    --gofast_out=plugins=grpc:. ${HAL_PROTO_FILES}

clean-proto:
    # Regenerate all .pb.go. This avoids conflicts due to file renames in nic/proto/hal
	rm -f ${GEN_DIR}/*.go

proto-sync: default
    # Will verify if the generated pb.go are committed
	@$(eval DIFFS=`git ls-files --exclude-standard --modified --directory ${CURDIR}`)
	@echo "Found the following uncommitted pb.go files, if any"
	@echo $(DIFFS)
	@test -z "$(DIFFS)"
