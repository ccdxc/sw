TOP_DIR = ../../../..
GEN_DIR = ${TOP_DIR}/nic/apollo/agent/gen/pds
LD_LIBRARY_PATH=$(TOP_DIR)/nic/hal/third-party/google/x86_64/lib/
HAL_PROTO_FILES = $(shell ls ${CURDIR} | grep .proto | grep -v meta.proto | grep -v types.proto | grep -v module | tr " " "\n" | LC_ALL=C sort | tr "\n" " ")
HAL_PB_GO_FILES = $(patsubst %.proto,%.pb.go,$(HAL_PROTO_FILES))
$(info ${HAL_PROTO_FILES})
$(info ${HAL_PB_GO_FILES})

default: create-gen-dir meta-proto pds-proto doc-gen
	echo 'goimports -w -l *.go'

pds-proto: clean-proto
	LC_ALL=C LD_LIBRARY_PATH=$(LD_LIBRARY_PATH) protoc --proto_path=${CURDIR} -I${CURDIR} \
                                -I${TOP_DIR} -I$(TOP_DIR)/vendor/github.com/gogo/protobuf/gogoproto \
                                -I$(TOP_DIR)/nic/hal/third-party/google/include -I${CURDIR}/meta \
				--gogofast_out=Mgogo.proto=github.com/gogo/protobuf/gogoproto,Mmeta/meta.proto=github.com/pensando/sw/nic/apollo/agent/gen/pds/meta/pds,plugins=grpc:${GEN_DIR} types.proto ${HAL_PROTO_FILES}

meta-proto:
	LC_ALL=C LD_LIBRARY_PATH=$(LD_LIBRARY_PATH) protoc --proto_path=${CURDIR} -I${CURDIR} \
                                -I${TOP_DIR} -I$(TOP_DIR)/vendor/github.com/gogo/protobuf/gogoproto \
                                -I$(TOP_DIR)/nic/hal/third-party/google/include -I${CURDIR}/meta \
                                --gogofast_out=Mgogo.proto=github.com/gogo/protobuf/gogoproto,Mgoogle/protobuf/timestamp.proto=github.com/gogo/protobuf/types,plugins=grpc:${GEN_DIR} meta.proto


create-gen-dir:
	mkdir -p ${GEN_DIR}
	mkdir -p ${CURDIR}/docs

doc-gen-clean:
	rm -f ${CURDIR}/docs/*

clean-proto:
	rm -f ${GEN_DIR}/*.go

gen-clean:
	rm -f ${GEN_DIR}/*.go
	
doc-gen: doc-gen-clean doc-gen-html doc-gen-md

doc-gen-html:
	LC_ALL=C LD_LIBRARY_PATH=$(LD_LIBRARY_PATH) protoc --proto_path=${CURDIR} -I${CURDIR} \
                                -I${TOP_DIR} -I$(TOP_DIR)/vendor/github.com/gogo/protobuf/gogoproto \
                                -I$(TOP_DIR)/nic/hal/third-party/google/include -I${CURDIR}/meta \
                                --doc_out=docs --doc_opt=html,docs.html types.proto ${HAL_PROTO_FILES}
doc-gen-md:
	LC_ALL=C LD_LIBRARY_PATH=$(LD_LIBRARY_PATH) protoc --proto_path=${CURDIR} -I${CURDIR} \
                                -I${TOP_DIR} -I$(TOP_DIR)/vendor/github.com/gogo/protobuf/gogoproto \
                                -I$(TOP_DIR)/nic/hal/third-party/google/include -I${CURDIR}/meta \
                                --doc_out=docs --doc_opt=markdown,docs.md types.proto ${HAL_PROTO_FILES}

proto-sync: default
	@$(eval DIFFS=`git ls-files --exclude-standard --modified --directory ${CURDIR}`)
	@echo "Found the following uncommitted pb.go files, if any"
	@echo $(DIFFS)
	@test -z "$(DIFFS)"
