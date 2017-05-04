#!/bin/bash

set -e
set -x

# set following variable to install docker distro
DOCKER_ENV=
if [ "$DOCKER_ENV" != "" ]; then
    sudo yum install -y yum-utils
    sudo yum-config-manager --add-repo https://download.docker.com/linux/centos/docker-ce.repo
    sudo yum-config-manager --enable docker-ce-edge
    sudo yum makecache fast
    sudo yum install -y docker-ce
    exit 0
fi

# Add kubernetes repo
cat <<END > /tmp/kubernetes.repo
[kubernetes]
name=Kubernetes
baseurl=http://yum.kubernetes.io/repos/kubernetes-el7-x86_64
enabled=1
gpgcheck=1
repo_gpgcheck=1
gpgkey=https://packages.cloud.google.com/yum/doc/yum-key.gpg
       https://packages.cloud.google.com/yum/doc/rpm-package-key.gpg
END

sudo cp /tmp/kubernetes.repo /etc/yum.repos.d/kubernetes.repo

# Install docker and kubernetes
sudo setenforce 0
sudo yum install -y docker kubelet kubeadm kubectl kubernetes-cni
sudo yum install -y wget

#disable selinux
sudo sed -i -e 's/--selinux-enabled//' /etc/sysconfig/docker
#jq is needed for the weave crash workaround on kubernetes
wget -q https://github.com/stedolan/jq/releases/download/jq-1.5/jq-linux64 && chmod +x jq-linux64 && sudo mv jq-linux64 /usr/local/bin


exit 0
