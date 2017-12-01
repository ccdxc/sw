all:
	@protoc -I/usr/local/include -I. -I${GOPATH}/src \
	-I../../vendor \
	-I${GOPATH}/src/github.com/pensando/sw/vendor/github.com/pensando/grpc-gateway/third_party \
	-I${GOPATH}/src/github.com/pensando/sw/vendor/github.com/pensando/grpc-gateway/third_party/googleapis \
	-I${GOPATH}/src/github.com/pensando/sw/vendor/github.com/gogo/protobuf/gogoproto \
	orch.proto --go_out=plugins=grpc:.
