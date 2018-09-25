default: pythongen
	protoc -I. -I../../vendor -I${GOPATH}/src -I../../vendor/github.com/pensando/grpc-gateway/third_party -I../../vendor/github.com/pensando/grpc-gateway/third_party/googleapis \
	-I../../vendor/github.com/gogo/protobuf/protobuf \
	--gofast_out=plugins=grpc:./gogen common.proto req_types.proto resp_types.proto cfg_svc.proto topo_svc.proto

pythongen:
	protoc -I. -I../../vendor -I${GOPATH}/src -I../../vendor/github.com/pensando/grpc-gateway/third_party -I../../vendor/github.com/pensando/grpc-gateway/third_party/googleapis \
    	-I../../vendor/github.com/gogo/protobuf/protobuf \
    	--python_out=plugins=grpc:./pygen common.proto req_types.proto resp_types.proto cfg_svc.proto topo_svc.proto
