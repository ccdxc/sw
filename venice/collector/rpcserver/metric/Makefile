all:
	@protoc -I. \
		-I../../../..//vendor \
		-I${GOPATH}/src \
		-I${GOPATH}/src/github.com/pensando/sw/vendor/github.com/pensando/grpc-gateway/third_party \
		-I${GOPATH}/src/github.com/pensando/sw/vendor/github.com/pensando/grpc-gateway/third_party/googleapis \
		-I${GOPATH}/src/github.com/pensando/sw/vendor/github.com/gogo/protobuf/protobuf \
		 metric.proto --go_out=plugins=grpc:.
	@goimports -local "github.com/pensando/sw" -l -w *.pb.go
