#!/bin/bash

cd tools/docker-files

if git diff-index --quiet HEAD --; then
    #no changes in tree. Tag by githash of tree and date
    VER=$(date +%Y%m%d.%H%M%S)-$(git rev-parse --short HEAD)
else
    # some local changes in the tree
    VER=$(date +%Y%m%d.%H%M%S)-local
fi
echo Building docker image with tag pen.io:${VER}
docker build --rm --no-cache -t pen.io:${VER} .
echo now you can run the image with syntax like:
echo docker run --privileged --net=host --name pen.io -v /sys/fs/cgroup:/sys/fs/cgroup:ro  -d pen.io:${VER}
