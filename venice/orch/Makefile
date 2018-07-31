all:
	@protoc -I. -I${GOPATH}/src \
	-I../../vendor \
	-I${GOPATH}/src/github.com/pensando/sw/vendor/github.com/pensando/grpc-gateway/third_party \
	-I${GOPATH}/src/github.com/pensando/sw/vendor/github.com/pensando/grpc-gateway/third_party/googleapis \
	-I${GOPATH}/src/github.com/pensando/sw/vendor/github.com/gogo/protobuf/gogoproto \
	-I${GOPATH}/src/github.com/pensando/sw/vendor/github.com/gogo/protobuf/protobuf \
	orch.proto --go_out=plugins=grpc:.
