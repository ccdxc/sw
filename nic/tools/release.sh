#!/bin/bash -e

VER=v1
IMAGE_DIR=obj/images
mkdir -p $IMAGE_DIR
if [ ! -f nic.tgz ]; then
    echo "nic.tgz file not found, please run \"make package\" first"
    exit 1
fi

\cp sim/naples/Dockerfile $IMAGE_DIR
\cp sim/naples/README $IMAGE_DIR
\cp sim/naples/Vagrantfile $IMAGE_DIR
\cp sim/naples/start-naples-docker.sh $IMAGE_DIR
\cp sim/naples/stop-naples-docker.sh $IMAGE_DIR
\mv nic.tgz $IMAGE_DIR

echo "Building and saving a docker image ..."
cd $IMAGE_DIR
docker build -t naples:$VER . && docker save -o naples-docker-$VER.tar naples:$VER
if [ $? -eq 0 ]; then
    gzip naples-docker-$VER.tar
    mv naples-docker-$VER.tar.gz naples-docker-$VER.tgz
    docker rmi -f naples:$VER
else
    echo "Failed to build docker image"
    \mv nic.tgz ../../
    exit $?
fi

# prepare the release tar ball now
echo "Preparing final image ..."
tar cvzf naples-release-$VER.tar.gz README Vagrantfile start-naples-docker.sh stop-naples-docker.sh naples-docker-$VER.tgz
\mv naples-release-$VER.tar.gz naples-release-$VER.tgz
\mv nic.tgz ../../
shopt -s extglob
rm -- !(naples-release-$VER.tgz)
cd -
echo "Image ready in $IMAGE_DIR"
exit 0
