#!/bin/bash

echo "++ generating schema for proto in local directory"
for protofile in *.proto
do
protoc -I. \
    -I${GOPATH}/src \
    -I${GOPATH}/src/github.com/pensando/sw/api \
    -I${GOPATH}/src/github.com/pensando/sw/vendor/github.com/pensando/grpc-gateway/third_party/googleapis \
    -I${GOPATH}/src/github.com/pensando/sw/vendor/github.com/pensando/grpc-gateway/third_party \
    -I${GOPATH}/src/github.com/pensando/sw/vendor/github.com/gogo/protobuf/protobuf \
    -I${GOPATH}/src/github.com/pensando/sw/vendor \
    --grpc-gateway_out=request_context=true,logtostderr=false,gengw=false,v=7,templates=github.com/pensando/sw/venice/utils/apigen/schema_local.yaml:. \
    ${protofile} || { echo "schema generation failed" ; exit -1; }
done