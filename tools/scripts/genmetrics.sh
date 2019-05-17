#!/bin/bash

if [ "$#" -ne 2 ]; then
    echo "Invalid number of arguments $# $0 $1 $2 $3 $4 "
    echo " $0 <prefix> <protobuf file directory>"
    exit 1
fi

prefix=$1
protopath=$2
curdir=$(pwd)

mkdir -p generated/${prefix}
echo "++ generating manifest" ${prefix} ${protopath} ${curdir}
cd ../${protopath}

for protofile in *.proto
do
    protoc -I. \
        -I${GOPATH}/src \
        -I${GOPATH}/src/github.com/pensando/sw/api/protos \
        -I${GOPATH}/src/github.com/pensando/sw/venice/utils/apigen/annotations \
        -I${GOPATH}/src/github.com/pensando/sw/vendor/github.com/pensando/grpc-gateway/third_party/googleapis \
        -I${GOPATH}/src/github.com/pensando/sw/vendor/github.com/pensando/grpc-gateway/third_party \
        -I${GOPATH}/src/github.com/pensando/sw/vendor/github.com/gogo/protobuf/protobuf \
        -I${GOPATH}/src/github.com/pensando/sw/vendor \
        -I ${GOPATH}/src/github.com/pensando/sw//nic/delphi/proto/delphi \
        --grpc-gateway_out=S_prefix=${prefix},S_path=${curdir},logtostderr=false,v=7,gengw=false,templates=github.com/pensando/sw/metrics/templates/pkgs.yaml,log_dir=${curdir}/tmp:${curdir}/generated/${prefix} \
        ${protofile} || { echo "manifest generation failed" ; exit -1; }
done

while read -r line || [[ -n "$line" ]];
do
    [[ -z "${line// }" ]] && continue
    echo "++ parsing ${line} for prefix ${prefix}"
    protoc -I. \
        -I${GOPATH}/src \
        -I${GOPATH}/src/github.com/pensando/sw/api \
        -I${GOPATH}/src/github.com/pensando/sw/vendor/github.com/pensando/grpc-gateway/third_party/googleapis \
        -I${GOPATH}/src/github.com/pensando/sw/vendor/github.com/pensando/grpc-gateway/third_party \
        -I${GOPATH}/src/github.com/pensando/sw/vendor/github.com/gogo/protobuf/protobuf \
        -I${GOPATH}/src/github.com/pensando/sw/vendor \
        -I${GOPATH}/src/github.com/pensando/sw/venice/utils/apigen/annotations \
        -I${GOPATH}/src/github.com/pensando/sw//nic/delphi/proto/delphi \
        --grpc-gateway_out=S_prefix=${prefix},S_path=${curdir},logtostderr=false,gengw=false,v=7,templates=github.com/pensando/sw/metrics/templates/config.yaml,log_dir=${curdir}/tmp:${curdir}/generated/${prefix} \
        ${line} || { echo "metrics generation failed" ; exit -1; }
done < ${curdir}/generated/${prefix}/metrics_files

cd ${curdir}