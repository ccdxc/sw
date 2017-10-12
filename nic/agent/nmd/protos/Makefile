default:
	protoc -I/usr/local/include -I. \
        -I../../../../vendor \
		-I${GOPATH}/src \
		-I../../../../vendor/github.com/grpc-ecosystem/grpc-gateway/third_party \
		-I../../../../vendor/github.com/grpc-ecosystem/grpc-gateway/third_party/googleapis \
		--gofast_out=Mgithub.com/pensando/sw/api/protos/cmd.proto=github.com/pensando/sw/api/generated/cmd,grpc:. nmd.proto
