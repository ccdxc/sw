#!/bin/bash

function createBaseContainer() {
    docker build --rm --no-cache -t pen-base:latest -f tools/docker-files/pencmd/Dockerfile tools/docker-files/pencmd
}

function createApiSrvContainer() {
    docker build --rm --no-cache -t pen-apiserver:latest -f tools/docker-files/apiserver/Dockerfile tools/docker-files/apiserver
}

function createApiGwContainer() {
    docker build --rm --no-cache -t pen-apigw:latest -f tools/docker-files/apigw/Dockerfile tools/docker-files/apigw
}

function createVCHubContainer() {
    docker build --rm --no-cache -t pen-vchub:latest -f tools/docker-files/vchub/Dockerfile tools/docker-files/vchub
}

function createNPMContainer() {
    docker build --rm --no-cache -t pen-npm:latest -f tools/docker-files/npm/Dockerfile tools/docker-files/npm
}

function createVCSimContainer() {
    docker build --rm --no-cache -t pen-vcsim:latest -f tools/docker-files/vcsim/Dockerfile tools/docker-files/vcsim
}

function createBinContainerTarBall() {
    staticimages="srv1.pensando.io:5000/google_containers/kube-controller-manager-amd64:v1.6.6 \
        srv1.pensando.io:5000/google_containers/kube-scheduler-amd64:v1.6.6 \
        srv1.pensando.io:5000/google_containers/kube-apiserver-amd64:v1.6.6 \
        srv1.pensando.io:5000/coreos/etcd:v3.2.1 srv1.pensando.io:5000/elasticsearch/elasticsearch:5.4.1 \
        srv1.pensando.io:5000/beats/filebeat:5.4.1 srv1.pensando.io:5000/pens-ntp:v0.2"
    for i in $staticimages
    do
        if [ "$(docker images -q $i)"  == "" ]
        then
            docker pull $i
        fi
    done
    mkdir -p bin/tars
    dynamicimages="pen-base:latest pen-apiserver:latest pen-apigw:latest pen-vchub:latest pen-npm:latest pen-vcsim:latest"
    for i in $staticimages $dynamicimages
    do
        # i is srv1.pensando.io:5000/google_containers/kube-controller-manager-amd64:v1.6.6
        # ${i##[^/]*/} is kube-controller-manager-amd64:v1.6.6
        # ${i##[^/]*/} | cut -d: -f1 is kube-controller-manager-amd64
        docker save -o bin/tars/$(echo ${i##[^/]*/} | cut -d: -f1).tar $i
    done
}

function startCluster() {
    vagrant ssh -c  '/import/src/github.com/pensando/sw/tools/scripts/startCluster.py -nodes ${PENS_NODES} -quorum ${PENS_QUORUM_NODENAMES}'  node1
}
function stopCluster() {
    vagrant ssh -c  '/import/src/github.com/pensando/sw/tools/scripts/startCluster.py -nodes ${PENS_NODES} -stop'  node1
}

case $1 in
    createBaseContainer) createBaseContainer ;;
    apigw) createApiGwContainer ;;
    apiserver) createApiSrvContainer;;
    vchub) createVCHubContainer;;
    npm) createNPMContainer;;
    vcsim) createVCSimContainer;;
    createBinContainerTarBall) createBinContainerTarBall;;
    startCluster) startCluster;;
    stopCluster) stopCluster ;;
    *) ;;
esac
