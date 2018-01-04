default:
	protoc -I/usr/local/include -I. \
        -I../../../../vendor \
		-I${GOPATH}/src \
		-I../../../../vendor/github.com/pensando/grpc-gateway/third_party \
        -I../../../../vendor/github.com/pensando/grpc-gateway/third_party/googleapis \
		--gofast_out=grpc:. agent.proto
