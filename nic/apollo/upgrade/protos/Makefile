TOP_DIR = ../../../..
UPG_PROTO_DIR = ${TOP_DIR}/nic/apollo/upgrade/protos
UPG_PROTO_FILE = upgrade.proto
GEN_DIR = ${TOP_DIR}/nic/apollo/agent/gen/pds
LD_LIBRARY_PATH=$(TOP_DIR)/nic/hal/third-party/google/x86_64/lib/

default: create-gen-dir upg-proto
	echo 'goimports -w -l *.go'

upg-proto: create-gen-dir
	LC_ALL=C LD_LIBRARY_PATH=$(LD_LIBRARY_PATH) protoc --proto_path=${UPG_PROTO_DIR} -I${UPG_PROTO_DIR} \
                                -I${TOP_DIR} -I$(TOP_DIR)/vendor/github.com/gogo/protobuf/gogoproto \
                                -I$(TOP_DIR)/nic/hal/third-party/google/include \
                                                --gogofast_out=Mgoogle/protobuf/any.proto=github.com/gogo/protobuf/types,Mgogo.proto=github.com/gogo/protobuf/gogoproto,plugins=grpc:${GEN_DIR} ${UPG_PROTO_FILE}

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

proto-sync: default
	@$(eval DIFFS=`git ls-files --exclude-standard --modified --directory ${CURDIR}`)
	@echo "Found the following uncommitted pb.go files, if any"
	@echo $(DIFFS)
	@test -z "$(DIFFS)"
