#!/bin/bash

set -e
set -x

if [ "$PACKER_BUILDER_TYPE" != "virtualbox-iso" ]; then
  exit 0
fi

sudo yum install -y net-tools
sudo yum install -y tcpdump
sudo yum install -y wget git

sudo yum group install -y "Development Tools"

# Install necessary packages for DPDK
sudo yum install -y gcc gcc-c++ CUnit-devel libaio-devel openssl-devel vim pciutils libtool curl unzip openssl numactl-devel python-paramiko
sudo yum install -y libibverbs-devel librdmacm-devel
sudo yum install -y kernel-devel glibc.i686 libgcc.i686 libstdc++.i686 glibc-devel.i686

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

#disable selinux on docker - else we keep seeing messages in dmesg
sudo sed -i -e 's/--selinux-enabled//' /etc/sysconfig/docker
#jq is needed for the weave crash workaround on kubernetes
wget -q https://github.com/stedolan/jq/releases/download/jq-1.5/jq-linux64 && chmod +x jq-linux64 && sudo mv jq-linux64 /usr/local/bin


cd /tmp
wget https://storage.googleapis.com/golang/go1.8.1.linux-amd64.tar.gz
sudo tar -C /usr/local -xzf go1.8.1.linux-amd64.tar.gz
rm go1.8.1.linux-amd64.tar.gz

# Install Swagger
sudo curl -o /usr/local/bin/swagger -L'#' https://github.com/go-swagger/go-swagger/releases/download/0.10.0/swagger_linux_amd64
sudo chmod +x /usr/local/bin/swagger

# install commonly used go tools
tee -a /tmp/goinstall.bash <<ENDF
export GOBIN=/usr/local/go/bin
export PATH=/usr/local/go/bin:$PATH
go get  "github.com/tools/godep"
go get  "github.com/nsf/gocode"
go get  "github.com/alecthomas/gometalinter"
go get  "golang.org/x/tools/cmd/goimports"
go get  "golang.org/x/tools/cmd/guru"
go get  "golang.org/x/tools/cmd/gorename"
go get  "github.com/golang/lint/golint"
go get  "github.com/rogpeppe/godef"
go get  "github.com/kisielk/errcheck"
go get  "github.com/jstemmer/gotags"
go get  "github.com/klauspost/asmfmt/cmd/asmfmt"
go get  "github.com/zmb3/gogetdoc"
go get  "github.com/josharian/impl"
gometalinter -i
ENDF

chmod +x /tmp/goinstall.bash
sudo bash -c /tmp/goinstall.bash

exit 0
