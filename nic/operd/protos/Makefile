TOP_DIR = ../../..
GEN_DIR = ${TOP_DIR}/nic/operd/daemon/gen/operd
LD_LIBRARY_PATH=$(TOP_DIR)/nic/hal/third-party/google/x86_64/lib/
OPERD_PROTO_FILES = $(shell ls ${CURDIR} | grep .proto | grep -v module | tr " " "\n" | LC_ALL=C sort | tr "\n" " ")
OPERD_PB_GO_FILES = $(patsubst %.proto,%.pb.go,$(OPERD_PROTO_FILES))
$(info ${OPERD_PROTO_FILES})
$(info ${OPERD_PB_GO_FILES})

default: create-gen-dir operd-proto doc-gen
	echo 'goimports -w -l *.go'

operd-proto: clean-proto
	LC_ALL=C LD_LIBRARY_PATH=$(LD_LIBRARY_PATH) protoc --proto_path=${CURDIR} -I${CURDIR} \
                                -I${TOP_DIR} -I$(TOP_DIR)/vendor/github.com/gogo/protobuf/gogoproto \
                                -I$(TOP_DIR)/nic/hal/third-party/google/include -I$(TOP_DIR)/nic/apollo/agent/protos \
                                -I$(TOP_DIR)/nic/apollo/agent/protos/meta \
                                --gogofast_out=Mgoogle/protobuf/any.proto=github.com/gogo/protobuf/types,Mgogo.proto=github.com/gogo/protobuf/gogoproto,Mtypes.proto=github.com/pensando/sw/nic/apollo/agent/gen/pds,Mmeta/meta.proto=github.com/pensando/sw/nic/apollo/agent/gen/pds/meta/pds,plugins=grpc:${GEN_DIR} ${OPERD_PROTO_FILES}

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
                                -I$(TOP_DIR)/nic/hal/third-party/google/include -I$(TOP_DIR)/nic/apollo/agent/protos -I$(TOP_DIR)/nic/apollo/agent/protos/meta \
                                --doc_out=docs --doc_opt=html,docs.html ${OPERD_PROTO_FILES}
doc-gen-md:
	LC_ALL=C LD_LIBRARY_PATH=$(LD_LIBRARY_PATH) protoc --proto_path=${CURDIR} -I${CURDIR} \
                                -I${TOP_DIR} -I$(TOP_DIR)/vendor/github.com/gogo/protobuf/gogoproto \
                                -I$(TOP_DIR)/nic/hal/third-party/google/include -I$(TOP_DIR)/nic/apollo/agent/protos -I$(TOP_DIR)/nic/apollo/agent/protos/meta \
                                --doc_out=docs --doc_opt=markdown,docs.md ${OPERD_PROTO_FILES}

proto-sync: default
	@$(eval DIFFS=`git ls-files --exclude-standard --modified --directory ${CURDIR}`)
	@echo "Found the following uncommitted pb.go files, if any"
	@echo $(DIFFS)
	@test -z "$(DIFFS)"
