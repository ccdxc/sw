#!/bin/bash

if [ "$PENS_NODES" == "" ]
then
    PENS_NODES=3
fi

function createBaseContainer() {
    if git diff-index --quiet HEAD --; then
        #no changes in tree. Tag by githash of tree and date
        VER=$(date +%Y%m%d.%H%M%S)-$(git rev-parse --short HEAD)
    else
        # some local changes in the tree
        VER=$(date +%Y%m%d.%H%M%S)-local
    fi
    echo Building docker image with tag pen-base:${VER}
    docker build --rm --no-cache -t pen-base:${VER} -f tools/docker-files/pencmd/Dockerfile tools/docker-files/pencmd
    docker tag pen-base:${VER} pen-base:latest
}

function createBinContainerTarBall() {
    if [ "$(docker images -q pen-ntp)"  == "" ] 
    then
        docker build --rm -t pen-ntp -f tools/docker-files/ntp/Dockerfile tools/docker-files/ntp
    fi
    images="gcr.io/google_containers/kube-controller-manager-amd64:v1.6.6 gcr.io/google_containers/kube-scheduler-amd64:v1.6.6 gcr.io/google_containers/kube-apiserver-amd64:v1.6.6 quay.io/coreos/etcd:v3.2.1"
    for i in $images
    do
        if [ "$(docker images -q $i)"  == "" ] 
        then
            docker pull $i
        fi
    done
    docker save -o bin/pen.tar pen-base:latest pen-ntp $images
}

function stopCluster() {
    ( 
        echo '#!/bin/bash -x'
        for j in pen-base pen-etcd pen-kube-controller-manager pen-kube-scheduler pen-kube-apiserver
        do
            echo "systemctl stop $j; docker stop $j ; docker rm $j" 
        done
        echo systemctl stop pen-kubelet 
        echo 'rm -fr /etc/pensando/* /etc/kubernetes/* /usr/pensando/bin/* /var/lib/pensando/*'
    ) > bin/node-cleanup
    chmod +x bin/node-cleanup    
    for i in $(seq 1 $PENS_NODES)
    do
        echo cleaning up node${i}
        vagrant ssh node${i} -- sudo bash -c /import/bin/node-cleanup > /dev/null 2>&1
        # TODO : stop other services/containers that get started by pensando
    done
}

function startCluster() {
    for i in $(seq 1 $PENS_NODES)
    do
        echo provisioning node${i}
        vagrant ssh node${i} -- docker load -i /import/bin/pen.tar
        vagrant ssh node${i} -- docker run --privileged --net=host --name pen-base -v /usr/pensando/bin:/host/usr/pensando/bin -v /usr/lib/systemd/system:/host/usr/lib/systemd/system -v /var/run/dbus:/var/run/dbus -v /run/systemd:/run/systemd  -v /etc/systemd/system:/etc/systemd/system  -v /etc/pensando:/etc/pensando -v /etc/kubernetes:/etc/kubernetes -v /sys/fs/cgroup:/sys/fs/cgroup:ro -d pen-base
    done
}

case $1 in 
    createBaseContainer) createBaseContainer ;;
    createBinContainerTarBall) createBinContainerTarBall;;
    startCluster) stopCluster ; startCluster;;
    stopCluster) stopCluster ;;
    *) ;;
esac
