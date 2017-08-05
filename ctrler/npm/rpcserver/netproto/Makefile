
default:
	protoc -I/usr/local/include -I. -I../../../../vendor -I${GOPATH}/src -I${GOPATH}/src/github.com/grpc-ecosystem/grpc-gateway/third_party -I${GOPATH}/src/github.com/grpc-ecosystem/grpc-gateway/third_party/googleapis \
	--gofast_out=Mgithub.com/pensando/sw/api/protos/network.proto=github.com/pensando/sw/api/generated/network,plugins=grpc:. network.proto endpoint.proto security.proto service.proto
