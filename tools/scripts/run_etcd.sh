#!/bin/bash

docker run -d -v /usr/share/ca-certificates/:/etc/ssl/certs --net=host \
 --name etcd quay.io/coreos/etcd:v3.2.13

# To check etcd state:
# docker exec -it etcd sh
# ETCDCTL_API=3 etcdctl get --prefix /venice/
