default:
	protoc -I/usr/local/include -I. \
        -I../../../../vendor \
		-I${GOPATH}/src \
		-I../../../../vendor/github.com/pensando/grpc-gateway/third_party \
        -I../../../../vendor/github.com/pensando/grpc-gateway/third_party/googleapis \
	-I../../../../vendor/github.com/gogo/protobuf/protobuf \
		--gofast_out=grpc:. agent.proto

gen-clean:
	rm -f agent.pb.go
