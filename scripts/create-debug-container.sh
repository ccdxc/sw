#!/bin/bash

set -ex

if [ "x$GOPATH" == "x" ]; then
  echo "GOPATH not set"
  exit 1
fi

base_dir=$GOPATH/src/github.com/pensando/sw
registry=registry.test.pensando.io:5000

mkdir -p $base_dir/bin/cbin/debug
cd $base_dir/bin/cbin/debug

# download kubectl
curl -fL --retry 3 --keepalive-time 2 -o kubernetes-server-linux-amd64.tar.gz https://storage.googleapis.com/kubernetes-release/release/v1.7.14/kubernetes-server-linux-amd64.tar.gz 
tar xvf kubernetes-server-linux-amd64.tar.gz
mv kubernetes/server/bin/kubectl .
chmod 755 ./kubectl
rm kubernetes-server-linux-amd64.tar.gz
rm -rf kubernetes

# download etcd
curl -fL --retry 3 --keepalive-time 2 -o etcd-linux-amd64.tar.gz https://storage.googleapis.com/etcd/v3.3.2/etcd-v3.3.2-linux-amd64.tar.gz
tar xvf etcd-linux-amd64.tar.gz
mv etcd-v3.3.2-linux-amd64/etcdctl .
chmod 755 ./etcdctl 
rm etcd-linux-amd64.tar.gz
rm -rf etcd-v3.3.2-linux-amd64

# build venice tools
go build -o ctctl github.com/pensando/sw/venice/citadel/cmd/ctctl
go build -o penctl github.com/pensando/sw/penctl
go build -o venice github.com/pensando/sw/venice/cli/cmd/venice

# build docker container
cd $base_dir
docker build -t $registry/pens-debug:v0.1 -f tools/docker-files/debug/Dockerfile .

exit 0
