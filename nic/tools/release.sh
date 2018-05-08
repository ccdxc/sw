#!/bin/bash -e

VER=v1
IMAGE_DIR=$(pwd)/obj/images

mkdir -p $IMAGE_DIR

if [ ! -f nic.tgz ]; then
    echo "nic.tgz file not found, please run \"make package\" first"
    exit 1
fi

ln -f nic.tgz sim/naples

echo "Building and saving a docker image ..."
cd sim/naples

docker build -t naples:$VER . && docker save -o naples-docker-$VER.tar naples:$VER
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
tar cvzf $IMAGE_DIR/naples-release-$VER.tgz README Vagrantfile naples-docker-$VER.tgz bootstrap.sh postman_env.json SF-Kingdom1.postman_collection.json SF-Kingdom2.postman_collection.json

echo "Image ready in $IMAGE_DIR"
exit 0
