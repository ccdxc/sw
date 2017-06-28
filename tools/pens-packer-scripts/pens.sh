#!/bin/bash

set -e
set -x

if [ "$PACKER_BUILDER_TYPE" != "virtualbox-iso" ]; then
  exit 0
fi

sudo yum install -y net-tools
sudo yum install -y tcpdump
sudo yum install -y wget
sudo yum group install -y "Development Tools"

if [ "$SWARM_ENV" != "" ]; then
  echo "**** building swarm env ****"
  exit 0
fi

cd /tmp
wget https://storage.googleapis.com/golang/go1.7.3.linux-amd64.tar.gz
sudo tar -C /usr/local -xzf go1.7.3.linux-amd64.tar.gz
rm go1.7.3.linux-amd64.tar.gz

# Install necessary packages for DPDK
sudo yum install -y gcc gcc-c++ CUnit-devel libaio-devel openssl-devel vim pciutils libtool curl unzip openssl numactl-devel
sudo yum install -y libibverbs-devel librdmacm-devel
sudo yum install -y kernel-devel glibc.i686 libgcc.i686 libstdc++.i686 glibc-devel.i686

# Install Swagger
sudo curl -o /usr/local/bin/swagger -L'#' https://github.com/go-swagger/go-swagger/releases/download/0.7.4/swagger_linux_amd64
sudo chmod +x /usr/local/bin/swagger

# Add kubernetes repo
cat <<EOF > /tmp/kubernetes.repo
[kubernetes]
name=Kubernetes
baseurl=http://yum.kubernetes.io/repos/kubernetes-el7-x86_64
enabled=1
gpgcheck=1
repo_gpgcheck=1
gpgkey=https://packages.cloud.google.com/yum/doc/yum-key.gpg
       https://packages.cloud.google.com/yum/doc/rpm-package-key.gpg
EOF

sudo cp /tmp/kubernetes.repo /etc/yum.repos.d/kubernetes.repo

# Install docker and kubernetes
sudo setenforce 0
sudo yum install -y docker kubelet kubeadm kubectl kubernetes-cni

# install protobuf related tools
mkdir -p ~/go && cd ~/go
export GOPATH=$PWD
mkdir -p $GOPATH/src/github.com/google
rm -rf protobuf
git clone https://github.com/google/protobuf.git
cd protobuf
./autogen.sh && ./configure && make
sudo make install && sudo ldconfig

# Install protoc-gen-go
go get -u github.com/golang/protobuf/protoc-gen-go
cp $GOPATH/bin/protoc-gen-go /usr/local/bin/
