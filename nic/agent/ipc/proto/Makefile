FWLOG_PROTO_DIR=${GOPATH}/src/github.com/pensando/sw/nic/proto/hal
FWLOG_OUT_DIR=$(PWD)
all:
	@cd ${FWLOG_PROTO_DIR}; protoc -I/usr/local/include -I. -I${GOPATH}/src \
	-I${GOPATH}/src/github.com/pensando/sw/vendor/github.com/gogo/protobuf/gogoproto \
	fwlog.proto --go_out=${FWLOG_OUT_DIR}
