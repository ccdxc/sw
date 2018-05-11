#!/bin/bash

function createCmdContainer() {
    docker build --rm --no-cache -t pen-cmd:latest -f tools/docker-files/cmd/Dockerfile tools/docker-files/cmd
}

function createApiSrvContainer() {
    docker build --rm --no-cache -t pen-apiserver:latest -f tools/docker-files/apiserver/Dockerfile tools/docker-files/apiserver
}

function createSpyglassContainer() {
    docker build --rm --no-cache -t pen-spyglass:latest -f tools/docker-files/spyglass/Dockerfile tools/docker-files/spyglass
}

function createApiGwContainer() {
    docker build --rm --no-cache -t pen-apigw:latest -f tools/docker-files/apigw/Dockerfile tools/docker-files/apigw
}

function createVCHubContainer() {
    docker build --rm --no-cache -t pen-vchub:latest -f tools/docker-files/vchub/Dockerfile tools/docker-files/vchub
}

function createNetagentContainer() {
    docker build --rm --no-cache -t pen-netagent:latest -f tools/docker-files/netagent/Dockerfile tools/docker-files/netagent
}

function createNmdContainer() {
    docker build --rm --no-cache -t pen-nmd:latest -f tools/docker-files/nmd/Dockerfile tools/docker-files/nmd
}

function createNPMContainer() {
    docker build --rm --no-cache -t pen-npm:latest -f tools/docker-files/pen-npm/Dockerfile tools/docker-files/pen-npm
}

function createTSMContainer() {
    docker build --rm --no-cache -t pen-tsm:latest -f tools/docker-files/tsm/Dockerfile tools/docker-files/tsm
}

function createVCSimContainer() {
    docker build --rm --no-cache -t pen-vcsim:latest -f tools/docker-files/vcsim/Dockerfile tools/docker-files/vcsim
}

function createCollectorContainer() {
    docker build --rm --no-cache -t pen-collector:latest -f tools/docker-files/collector/Dockerfile tools/docker-files/collector
}

function createTpmContainer() {
    docker build --rm --no-cache -t pen-tpm:latest -f tools/docker-files/tpm/Dockerfile tools/docker-files/tpm
}

function createEvtsmgrContainer() {
    docker build --rm --no-cache -t pen-evtsmgr:latest -f tools/docker-files/evtsmgr/Dockerfile tools/docker-files/evtsmgr
}

function createBinContainerTarBall() {
    staticimages="registry.test.pensando.io:5000/google_containers/kube-controller-manager-amd64:v1.7.14 \
        registry.test.pensando.io:5000/google_containers/kube-scheduler-amd64:v1.7.14 \
        registry.test.pensando.io:5000/google_containers/kube-apiserver-amd64:v1.7.14 \
        registry.test.pensando.io:5000/coreos/etcd:v3.3.2 registry.test.pensando.io:5000/beats/filebeat:6.2.2 \
        registry.test.pensando.io:5000/pens-ntp:v0.2 registry.test.pensando.io:5000/influxdb:1.4.2 \
        registry.test.pensando.io:5000/elasticsearch-cluster:v0.2"
    for i in $staticimages
    do
        if [ "$(docker images -q $i)"  == "" ]
        then
            docker pull $i
        fi
    done
    mkdir -p bin/tars
    dynamicimages="pen-cmd:latest pen-apiserver:latest pen-apigw:latest pen-vchub:latest \
        pen-npm:latest pen-vcsim:latest pen-netagent:latest pen-nmd:latest pen-collector:latest \
        pen-tpm:latest pen-spyglass:latest pen-evtsmgr:latest pen-tsm:latest"
    for i in $staticimages $dynamicimages
    do
        # i is registry.test.pensando.io:5000/google_containers/kube-controller-manager-amd64:v1.6.6
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
    cmd) createCmdContainer ;;
    apigw) createApiGwContainer ;;
    apiserver) createApiSrvContainer;;
    vchub) createVCHubContainer;;
    pen-npm) createNPMContainer;;
    vcsim) createVCSimContainer;;
    netagent) createNetagentContainer;;
    nmd) createNmdContainer;;
    collector) createCollectorContainer;;
    tpm) createTpmContainer;;
    spyglass) createSpyglassContainer;;
    evtsmgr) createEvtsmgrContainer;;
    tsm) createTSMContainer;;
    createBinContainerTarBall) createBinContainerTarBall;;
    startCluster) startCluster;;
    stopCluster) stopCluster ;;
    *) ;;
esac
