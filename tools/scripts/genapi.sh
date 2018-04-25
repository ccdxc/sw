#!/bin/bash
if [ "$#" -ne 1 ]; then
    echo "Invalid number of arguments"
    echo " $0 <protobuf file directory>"
    exit 1
fi
protopath=$1

if [ ! -e ${protopath} ]; then
    echo "File ${protopath} not found"
    exit 1
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

if [ -e ${curdir}/generated/apiclient/svcmanifest.json ]; then
    rm -f ${curdir}/generated/apiclient/svcmanifest.json
fi

echo "++ generating manifest"
cd ${curdir}/${protopath}
echo "changed path to ${curdir}/${protopath} - $(pwd)"
for protofile in *.proto
do
    protoc -I/usr/local/include -I. \
        -I${GOPATH}/src \
        -I${GOPATH}/src/github.com/pensando/sw/api/protos \
        -I${GOPATH}/src/github.com/pensando/sw/vendor/github.com/pensando/grpc-gateway/third_party/googleapis \
        -I${GOPATH}/src/github.com/pensando/sw/vendor/github.com/pensando/grpc-gateway/third_party \
        -I${GOPATH}/src/github.com/pensando/sw/vendor \
        --grpc-gateway_out=logtostderr=false,v=7,gengw=false,templates=github.com/pensando/sw/venice/utils/apigen/manifest.yaml,log_dir=${curdir}/tmp:${curdir}/generated/ \
        ${protofile} || { echo "manifest generation failed" ; exit -1; }
done

cd ${curdir}
cd ${curdir}/${protopath}
while read -r line || [[ -n "$line" ]];
do
    protofiles=$(echo $line | awk  '{ for (i = 3; i <= NF; i++) { printf("%s ", $i) } }')
    pkg=$(echo $line | awk '{ print $2 }')
    [[ -z "${protofile// }" ]] || [[ -z "${pkg// }" ]] && continue
    echo "++ parsing ${protofiles} for pkg ${pkg}"
    protoc -I/usr/local/include -I. \
        -I${GOPATH}/src \
        -I${GOPATH}/src/github.com/pensando/sw/api \
        -I${GOPATH}/src/github.com/pensando/sw/vendor/github.com/pensando/grpc-gateway/third_party/googleapis \
        -I${GOPATH}/src/github.com/pensando/sw/vendor/github.com/pensando/grpc-gateway/third_party \
        -I${GOPATH}/src/github.com/pensando/sw/vendor \
        --grpc-gateway_out=request_context=true,logtostderr=false,gengw=true,v=7,templates=github.com/pensando/sw/venice/utils/apigen/config.yaml,log_dir=${curdir}/tmp:${curdir}/generated/${pkg} \
        ${protofiles} || { echo "grpc-gateway generation failed" ; exit -1; }
done < ${curdir}/generated/pkgmanifest

cd ${curdir}
PROTOSUBST=Mgoogle/api/annotations.proto=github.com/pensando/grpc-gateway/third_party/googleapis/google/api,Mgoogle/protobuf/timestamp.proto=github.com/gogo/protobuf/types
cd $protopath && while read -r line || [[ -n "$line" ]];
do
    protofile=$(echo $line | awk '{ print $1 }')
    pkg=$(echo $line | awk '{ print $2 }')
    [[ -z "${protofile// }" ]] || [[ -z "${pkg// }" ]] && continue
    PROTOSUBST=${PROTOSUBST},Mgithub.com/pensando/sw/api/protos/${protofile}=github.com/pensando/sw/api/generated/${pkg}
done < ${curdir}/generated/manifest

cd ${curdir}
# cd ${curdir}/${protopath}
cd $protopath && while read -r line || [[ -n "$line" ]];
do
    protofiles=$(echo $line | awk '{ for (i = 3; i <= NF; i++) { printf("%s ", $i) } }')
    pkg=$(echo $line | awk '{ print $2 }')
    [[ -z "${protofile// }" ]] || [[ -z "${pkg// }" ]] && continue
    echo "++ parsing ${protofiles} for pkg ${pkg}"

    protoc -I/usr/local/include -I. \
        -I${GOPATH}/src \
        -I${GOPATH}/src/github.com/pensando/sw/api/protos \
        -I${GOPATH}/src/github.com/pensando/sw/vendor/github.com/pensando/grpc-gateway/third_party/googleapis \
        -I${GOPATH}/src/github.com/pensando/sw/vendor/github.com/pensando/grpc-gateway/third_party \
        -I${GOPATH}/src/github.com/pensando/sw/vendor \
        --swagger_out=logtostderr=true:${curdir}/generated/${pkg}/swagger \
        ${protofiles} || { echo "swagger generation failed" ; exit -1; }
    protoc -I/usr/local/include -I. \
        -I${GOPATH}/src \
        -I${GOPATH}/src/github.com/pensando/sw/api/protos \
        -I${GOPATH}/src/github.com/pensando/sw/vendor/github.com/pensando/grpc-gateway/third_party/googleapis \
        -I${GOPATH}/src/github.com/pensando/sw/vendor/github.com/pensando/grpc-gateway/third_party \
        -I${GOPATH}/src/github.com/pensando/sw/vendor \
        --pensando_out=plugins=grpc,logtostderr=false,log_dir=${curdir}/tmp,${PROTOSUBST}:${curdir}/generated/${pkg} \
        ${protofiles} || { echo "Protobuf generation failed" ; exit -1; }
        echo ++ Generating swagger for ${curdir}/generated/${pkg}/gateway
        tempdir=$(pwd)&& cd ${curdir}/generated/${pkg}/gateway && rice embed-go && go generate .
        cd $tempdir
done < ${curdir}/generated/pkgmanifest
cd ${curdir}
echo "++ running goimports"
# Go format code
goimports -local "github.com/pensando/sw" -l -w generated
