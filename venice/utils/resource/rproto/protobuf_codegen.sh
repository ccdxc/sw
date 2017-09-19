#!/bin/bash

# This script runs the protobuf compiler to generate golang bindings
# use it as: codegen.sh ./<your .proto file>
# Its preferred that you run this from the directory where .proto file is present
#
proto=./rsrc.proto 
protoc -I. -I$GOPATH/src -I../../../vendor --go_out=plugins=grpc:. $proto
