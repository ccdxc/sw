#!/bin/sh

if [ "$#" -ne 1 ]; then
    echo "Invalid number of arguments"
    echo " $0 <protobuf file directory>"
    exit -1
fi
protopath=$1

if [ ! -e ${protopath} ]; then
    echo "File ${protopath} not found"
    exit -1
fi

curdir=$(pwd)
mkdir -p ${curdir}/generated

# Delete the manifest file if already exists.
if [ -e ${curdir}/generated/manifest ]; then
    rm -f ${curdir}/generated/manifest
fi

# Delete the relations file if already exists.
if [ -e ${curdir}/generated/relations.json ]; then
    rm -f ${curdir}/generated/relations.json
fi

echo "++ generating manifest"
for protofile in ${protopath}/*.proto
do
    protoc -I/usr/local/include -I. \
        -I${GOPATH}/src \
        -I${GOPATH}/src/github.com/pensando/sw/vendor/github.com/pensando/grpc-gateway/third_party/googleapis \
        -I${GOPATH}/src/github.com/pensando/sw/vendor/github.com/pensando/grpc-gateway/third_party \
        -I${GOPATH}/src/github.com/pensando/sw/vendor \
        --grpc-gateway_out=logtostderr=false,v=7,gengw=false,templates=github.com/pensando/sw/utils/apigen/manifest.yaml,log_dir=${curdir}/tmp:${curdir}/generated/ \
        ${protofile} || { echo "grpc-gateway generation failed" ; exit -1; }
done

cd ${curdir}
while read -r line || [[ -n "$line" ]];
do
    protofile=$(echo $line | awk '{ print $1 }')
    pkg=$(echo $line | awk '{ print $2 }')
    [[ -z "${protofile// }" ]] || [[ -z "${pkg// }" ]] && continue
    echo "++ parsing ${protofile} for pkg ${pkg}"
    protoc -I/usr/local/include -I. \
        -I${GOPATH}/src \
        -I${GOPATH}/src/github.com/pensando/sw/vendor/github.com/pensando/grpc-gateway/third_party/googleapis \
        -I${GOPATH}/src/github.com/pensando/sw/vendor/github.com/pensando/grpc-gateway/third_party \
        -I${GOPATH}/src/github.com/pensando/sw/vendor \
        --grpc-gateway_out=request_context=true,logtostderr=false,gengw=true,v=7,templates=github.com/pensando/sw/utils/apigen/config.yaml,log_dir=${curdir}/tmp:${curdir}/generated/${pkg} \
        ${protopath}/${protofile} || { echo "grpc-gateway generation failed" ; exit -1; }
done < ${curdir}/generated/manifest

cd ${curdir}
PROTOSUBST=Mgoogle/api/annotations.proto=github.com/pensando/grpc-gateway/third_party/googleapis/google/api
cd $protopath && while read -r line || [[ -n "$line" ]];
do
    protofile=$(echo $line | awk '{ print $1 }')
    pkg=$(echo $line | awk '{ print $2 }')
    [[ -z "${protofile// }" ]] || [[ -z "${pkg// }" ]] && continue
    PROTOSUBST=${PROTOSUBST},Mgithub.com/pensando/sw/api/protos/${protofile}=github.com/pensando/sw/api/generated/${pkg}
done < ${curdir}/generated/manifest

cd ${curdir}
cd $protopath && while read -r line || [[ -n "$line" ]];
do
    protofile=$(echo $line | awk '{ print $1 }')
    pkg=$(echo $line | awk '{ print $2 }')
    [[ -z "${protofile// }" ]] || [[ -z "${pkg// }" ]] && continue
    echo "++ parsing ${protofile} for pkg ${pkg}"

    protoc -I/usr/local/include -I. \
        -I${GOPATH}/src \
        -I${GOPATH}/src/github.com/pensando/sw/vendor/github.com/pensando/grpc-gateway/third_party/googleapis \
        -I${GOPATH}/src/github.com/pensando/sw/vendor/github.com/pensando/grpc-gateway/third_party \
        -I${GOPATH}/src/github.com/pensando/sw/vendor \
        --swagger_out=logtostderr=true:../generated/${pkg}/swagger \
        ${protofile} || { echo "swagger generation failed" ; exit -1; }
    protoc -I/usr/local/include -I. \
        -I${GOPATH}/src \
        -I${GOPATH}/src/github.com/pensando/sw/vendor/github.com/pensando/grpc-gateway/third_party/googleapis \
        -I${GOPATH}/src/github.com/pensando/sw/vendor/github.com/pensando/grpc-gateway/third_party \
        -I${GOPATH}/src/github.com/pensando/sw/vendor \
        --gofast_out=plugins=grpc,${PROTOSUBST}:${curdir}/generated/${pkg} \
        ${protofile} || { echo "Protobuf generation failed" ; exit -1; }
        echo ++ Generating swagger for ${curdir}/generated/${pkg}/gateway
        tempdir=$(pwd)&& cd ${curdir}/generated/${pkg}/gateway && rice embed-go && go generate .
        cd $tempdir
done < ${curdir}/generated/manifest
cd ${curdir}
echo "++ running go fmt"
# Go format code
gofmt -w generated
