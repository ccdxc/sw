#!/bin/bash

if [ "$UID" != "0" ]; then
   #$0 is the script itself (or the command used to call it)...
   #$* parameters...
     sudo $0 $*
     exit
fi

set -x # temporarily enable debugging to figure out iota install issue

# this script is run by customer after extracting the image.
# this is supposed to install all the venice components in the system

# cleanup any installation on this node as if we are gonna install fresh again
function cleanupNode() {
        for srv in "pen-cmd" "pen-etcd" "pen-kube-controller-manager" "pen-kube-scheduler" "pen-kube-apiserver"
        do
            systemctl stop  ${srv}
            systemctl disable  ${srv}
        done
        systemctl stop pen-kubelet
        for i in $(/usr/bin/systemctl list-unit-files --no-legend --no-pager -l | grep --color=never -o kube.*\.slice )
        do
            systemctl stop $i
        done
        if [ "$(docker ps -qa)" != "" ]
        then
            docker stop -t 2 $(docker ps -qa)
            docker rm $(docker ps -qa)
        fi
        for i in $(cat /proc/mounts | grep kubelet | cut -d " " -f 2)
        do
            umount $i
        done
        rm -fr /etc/pensando/* /etc/kubernetes/* /usr/pensando/bin/* /var/lib/pensando/* /var/log/pensando/*  /var/lib/cni/ /var/lib/kubelet/* /etc/cni/ /data/minio/*
        ip addr flush label *pens
        if [ "$(docker ps -qa)" != "" ]
        then
            docker stop -t 2 $(docker ps -qa)
            docker rm -f $(docker ps -qa)
        fi
}

function elasticSysctl() {
    TARGET_KEY="vm.max_map_count"
    CONFIG_FILE="/etc/sysctl.conf"
    REPLACEMENT_VALUE=262144

    if grep -q "^[ 	]*$TARGET_KEY[ 	]*=" "$CONFIG_FILE"; then
        sed -i -e "s^\\([ 	]*$TARGET_KEY[ 	]*=[ 	]*\\).*$\\1$REPLACEMENT_VALUE" "$CONFIG_FILE"
    else
        echo "$TARGET_KEY = $REPLACEMENT_VALUE" | tee -a $CONFIG_FILE
    fi
    sysctl -p
}

function disableNTP() {
    systemctl stop chronyd || echo
    systemctl disable chronyd || echo
    systemctl stop systemd-timesyncd.service || echo
    systemctl disable systemd-timesyncd.service || echo
}

function clockSettings() {
     vmware-toolbox-cmd timesync disable || echo
     timedatectl set-local-rtc 0
     hwclock -wu
}

function dockerSettings() {
    mkdir -p /etc/docker
    if [ ! -f /etc/docker/daemon.json ] ; then
        systemctl stop docker || :
        printf "{\n\t\"default-ipc-mode\": \"shareable\",\n\t\"log-driver\": \"journald\"\n}" > /etc/docker/daemon.json
        systemctl start docker || :
    elif ! grep 'default-ipc-mode' /etc/docker/daemon.json && ! grep 'log-driver' /etc/docker/daemon.json; then
        systemctl stop docker || :
        sed -i -e "s{{\n\t\"default-ipc-mode\": \"shareable\",\n\t\"log-driver\": \"journald\",\n" /etc/docker/daemon.json
        systemctl start docker || :
    elif ! grep 'default-ipc-mode' /etc/docker/daemon.json ; then
        systemctl stop docker || :
        sed -i -e "s{{\n\t\"default-ipc-mode\": \"shareable\",\n" /etc/docker/daemon.json
        systemctl start docker || :
    elif ! grep 'log-driver' /etc/docker/daemon.json ; then
        systemctl stop docker || :
        sed -i -e "s{{\n\t\"log-driver\": \"journald\",\n" /etc/docker/daemon.json
        systemctl start docker || :
    fi
}

if [ "$1" == "--clean" ]
then
    cleanupNode
fi

if [ "$1" == "--clean-only" ]
then
    dockerSettings
    cleanupNode
    exit 0
fi

# on some systems systemctl is in /bin/systemctl
if  [ ! -f /usr/bin/systemctl  -a -f /bin/systemctl ]
then
    sudo ln -s /bin/systemctl /usr/bin/systemctl
fi

elasticSysctl
disableNTP
clockSettings
dockerSettings

mkdir -p /data /run/initramfs/live /usr/local/bin

for i in tars/pen* ; do docker load -i  $i; done
docker run --rm --name pen-install -v /var/log/pensando:/host/var/log/pensando -v /var/lib/pensando:/host/var/lib/pensando -v /usr/pensando/bin:/host/usr/pensando/bin -v /usr/lib/systemd/system:/host/usr/lib/systemd/system -v /etc/pensando:/host/etc/pensando pen-install -c /initscript

systemctl daemon-reload
systemctl enable pensando.target
systemctl start pensando.target
systemctl enable pen-cmd
systemctl start pen-cmd
exit 0
