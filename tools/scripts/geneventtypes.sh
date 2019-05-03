#!/bin/bash

get_package_name() {
    while read -r line || [[ -n "$line" ]];
    do
        pkg=$(echo $line | grep "package" | awk '{ print $2 }' | sed 's/.$//')
        [[ ! -z "$pkg" ]] && echo ${pkg} && return || continue
    done < $1
}

if [ "$#" -ne 1 ]; then
    echo "Invalid number of arguments"
    echo " $0 <protobuf file directory>"
    exit 1
fi
protopath=$1

if [ ! -e ${protopath} ]; then
    echo "Directory ${protopath} not found"
    exit 1
fi

## make sw/events/generated
evtsdir=$(pwd)
mkdir -p ${evtsdir}/generated

## cd sw/events/protos
protopath=${evtsdir}/${protopath}
cd ${protopath}

## generate *.pb.go files
echo "++ generating event types"
for protofile in *.proto
do
    pkg=$(get_package_name $protofile)
    mkdir -p ${evtsdir}/generated/${pkg}
    protoc -I. \
        -I${GOPATH}/src \
        -I${GOPATH}/src/github.com/pensando/sw/api \
        -I${GOPATH}/src/github.com/pensando/sw/events \
        -I${GOPATH}/src/github.com/pensando/sw/vendor/github.com/pensando/grpc-gateway/third_party/googleapis \
        -I${GOPATH}/src/github.com/pensando/sw/vendor/github.com/gogo/protobuf/gogoproto \
        -I${GOPATH}/src/github.com/pensando/sw/vendor/github.com/gogo/protobuf/protobuf \
        -I${GOPATH}/src/github.com/pensando/sw/vendor \
        --pensando_out=Mattributes.proto=github.com/pensando/sw/events/generated/eventattrs,Mgoogle/protobuf/descriptor.proto=github.com/gogo/protobuf/protoc-gen-gogo/descriptor,Mgithub.com/pensando/sw/api/protos/events.proto=github.com/pensando/sw/api/generated/events,Mgogo.proto=github.com/gogo/protobuf/gogoproto,plugins=grpc:${evtsdir}/generated/${pkg} \
        ${protofile} || { echo "Event types generation failed" ; exit -1; }
done

## generate *.go files
protofile=${evtsdir}/protos/eventtypes.proto
pkg=$(get_package_name $protofile)
protoc -I. \
    -I${GOPATH}/src \
    -I${GOPATH}/src/github.com/pensando/sw/events \
    -I${GOPATH}/src/github.com/pensando/sw/vendor/github.com/pensando/grpc-gateway/third_party/googleapis \
    -I${GOPATH}/src/github.com/pensando/sw/vendor/github.com/gogo/protobuf/gogoproto \
    -I${GOPATH}/src/github.com/pensando/sw/vendor/github.com/gogo/protobuf/protobuf \
    -I${GOPATH}/src/github.com/pensando/sw/vendor \
    --grpc-gateway_out=logtostderr=false,v=7,gengw=false,log_dir=${evtsdir}/tmp,templates=github.com/pensando/sw/events/manifest.yaml:${evtsdir}/generated/${pkg} \
    ${protofile} || { echo "Event types generation failed" ; exit -1; }

## fix imports in /sw/events/generated/eventtypes/*.pb.go
genfiles=${evtsdir}/generated/eventtypes
cd ${genfiles}
sed -i.bak_ 's/import _ .*//' *.pb.go
rm *.bak_

## fix imports in /sw/events/generated/eventattrs/*.pb.go
genfiles=${evtsdir}/generated/eventattrs
cd ${genfiles}
sed -i.bak_ 's/import _ .*//' *.pb.go
rm *.bak_

cd ${evtsdir}

echo "++ running goimports"
goimports -local "github.com/pensando/sw" -l -w ${evtsdir}
