all:
	@protoc -I/usr/local/include -I. \
		-I../../../..//vendor \
		-I${GOPATH}/src \
		-I${GOPATH}/src/github.com/pensando/sw/vendor/github.com/pensando/grpc-gateway/third_party \
		-I${GOPATH}/src/github.com/pensando/sw/vendor/github.com/pensando/grpc-gateway/third_party/googleapis \
		 metric.proto --go_out=plugins=grpc:.
