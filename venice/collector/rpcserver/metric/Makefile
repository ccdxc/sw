all:
	protoc -I/usr/local/include -I. -I../../../../vendor -I${GOPATH}/src -I../../../../vendor/github.com/grpc-ecosystem/grpc-gateway/third_party -I../../../../vendor/github.com/grpc-ecosystem/grpc-gateway/third_party/googleapis metric.proto --go_out=plugins=grpc:.
