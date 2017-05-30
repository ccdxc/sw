#!/bin/sh

if [ "$#" -ne 1 ]; then
    echo "Invalid number of arguments"
    echo " $0 <protobuf file>"
    exit -1
fi
protofile=$1

if [ ! -e ${protofile} ]; then
    echo "File ${protofile} not found"
    exit -1
fi

# create needed directories
DIRS=( generated/grpc/client generated/grpc/server generated/gateway generated/swagger impl )
for d in "${DIRS[@]}"; do
    mkdir -p "${d}"
done

protoc -I/usr/local/include -I. \
    -I${GOPATH}/src \
    -I${GOPATH}/src/github.com/grpc-ecosystem/grpc-gateway/third_party/googleapis \
    -I${GOPATH}/src/github.com/grpc-ecosystem/grpc-gateway/third_party \
    -I${GOPATH}/src/github.com/pensando/sw/vendor \
    --gofast_out=Mgoogle/api/annotations.proto=github.com/grpc-ecosystem/grpc-gateway/third_party/googleapis/google/api,plugins=grpc:./generated/ \
    ${protofile} || { echo "Protobuf generation failed" ; exit -1; }
protoc -I/usr/local/include -I. \
    -I${GOPATH}/src \
    -I${GOPATH}/src/github.com/grpc-ecosystem/grpc-gateway/third_party/googleapis \
    -I${GOPATH}/src/github.com/grpc-ecosystem/grpc-gateway/third_party \
    -I${GOPATH}/src/github.com/pensando/sw/vendor \
    --grpc-gateway_out=logtostderr=false,v=7,templates=github.com/pensando/sw/utils/apigen/config.yaml,log_dir=./tmp:./generated/ \
    ${protofile} || { echo "grpc-gateway generation failed" ; exit -1; }
protoc -I/usr/local/include -I. \
    -I${GOPATH}/src \
    -I${GOPATH}/src/github.com/grpc-ecosystem/grpc-gateway/third_party/googleapis \
    -I${GOPATH}/src/github.com/grpc-ecosystem/grpc-gateway/third_party \
    -I${GOPATH}/src/github.com/pensando/sw/vendor \
    --swagger_out=logtostderr=true:./generated/swagger \
    ${protofile} || { echo "swagger generation failed" ; exit -1; }

go generate ./generated/
cd ./generated/gateway  && rice embed-go && cd ../../

# Go format code
gofmt -l -w generated
