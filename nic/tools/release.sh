#!/bin/bash -e

VER=v1
IMAGE_DIR=$(pwd)/obj/images

mkdir -p $IMAGE_DIR

NIC_TAR_NAME=nic_${ARCH}_${PIPELINE}_${ASIC}
echo "release: NIC_TAR_NAME=$NIC_TAR_NAME, ARCH=$ARCH, PIPELINE=$PIPELINE, ASIC=$ASIC"

if [ ! -f ${NIC_TAR_NAME}.tgz ]; then
    echo "${NIC_TAR_NAME}.tgz file not found, please run \"make package\" first"
    exit 1
fi

case "$1" in
    'apulu-venice')
        echo "Proceeding to build docker image for apulu-venice pipeline"
        ln -f ${NIC_TAR_NAME}_venice.tgz sim/naples/nic.tgz
        cd sim/naples
        SUPPORT_FILES="README Vagrantfile naples_vm_bringup.py"
        ;;
    *)
        echo "Proceeding to build docker image for iris pipeline"
        ln -f ${NIC_TAR_NAME}.tgz sim/naples/nic.tgz
        #ln -f debug.tgz sim/naples
        cd sim/naples
        SUPPORT_FILES="README Vagrantfile naples_vm_bringup.py"
        ;;
esac

docker build -t pensando/naples:$VER . && docker save -o naples-docker-$VER.tar pensando/naples:$VER
if [ $? -eq 0 ]; then
    gzip naples-docker-$VER.tar
    mv naples-docker-$VER.tar.gz naples-docker-$VER.tgz
    dangling_images=$(docker images -qa -f 'dangling=true')
    if [ "x$dangling_images" != "x" ]; then
        docker rmi -f $dangling_images
    fi
else
    echo "Failed to build docker image"
    exit $?
fi

# prepare the release tar ball now
echo "Preparing final image ..."
tar cvzf $IMAGE_DIR/naples-release-$VER.tgz naples-docker-$VER.tgz $SUPPORT_FILES

echo "Image ready in $IMAGE_DIR"
exit 0
