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


function createBinContainerTarBall() {
    images="srv1.pensando.io:5000/google_containers/kube-controller-manager-amd64:v1.6.6 \
        srv1.pensando.io:5000/google_containers/kube-scheduler-amd64:v1.6.6 \
        srv1.pensando.io:5000/google_containers/kube-apiserver-amd64:v1.6.6 \
        srv1.pensando.io:5000/coreos/etcd:v3.2.1 srv1.pensando.io:5000/elasticsearch/elasticsearch:5.4.1 \
        srv1.pensando.io:5000/beats/filebeat:5.4.1 srv1.pensando.io:5000/pens-ntp:v0.2"
    for i in $images
    do
        if [ "$(docker images -q $i)"  == "" ] 
        then
            docker pull $i
        fi
    done
    docker save -o bin/pen.tar pen-base:latest pen-apiserver:latest pen-apigw:latest $images
}

function startCluster() {    
    vagrant ssh -c  '/import/src/github.com/pensando/sw/tools/scripts/startCluster.py -nodes ${PENS_NODES}'  node1
}
function stopCluster() {
    vagrant ssh -c  '/import/src/github.com/pensando/sw/tools/scripts/startCluster.py -nodes ${PENS_NODES} -stop'  node1
}

case $1 in 
    createBaseContainer) createBaseContainer ;;
    createApiGwContainer) createApiGwContainer ;;
    createApiSrvContainer) createApiSrvContainer;;
    createBinContainerTarBall) createBinContainerTarBall;;
    startCluster) startCluster;;
    stopCluster) stopCluster ;;
    *) ;;
esac
