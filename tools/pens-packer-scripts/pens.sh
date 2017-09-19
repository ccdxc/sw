#!/bin/bash

set -e
set -x

if [ "$PACKER_BUILDER_TYPE" != "virtualbox-iso" ]; then
  exit 0
fi

if [ "$SWARM_ENV" != "" ]; then
  echo "**** building swarm env ****"
  exit 0
fi


## Add all the repositories
sudo yum install -y yum-utils
sudo yum-config-manager  -y --add-repo https://download.docker.com/linux/centos/docker-ce.repo
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

sudo yum -y makecache fast

sudo setenforce 0


sudo yum remove -y docker docker-common  docker-selinux docker-engine

sudo yum group install -y "Development Tools"
sudo yum install -y net-tools tcpdump wget git gcc gcc-c++ CUnit-devel  \
    openssl-devel vim pciutils libtool curl unzip openssl  \
    python-paramiko kernel-devel glibc.i686 libgcc.i686 \
    libstdc++.i686 glibc-devel.i686 kubelet kubeadm kubectl kubernetes-cni \
    device-mapper-persistent-data lvm2 docker-ce libpcap-devel



cd /tmp
wget https://storage.googleapis.com/golang/go1.8.3.linux-amd64.tar.gz
sudo tar -C /usr/local -xzf go1.8.3.linux-amd64.tar.gz
rm go1.8.3.linux-amd64.tar.gz


# Install Swagger
sudo curl -o /usr/local/bin/swagger -L'#' https://github.com/go-swagger/go-swagger/releases/download/0.7.4/swagger_linux_amd64
sudo chmod +x /usr/local/bin/swagger

#jq is needed for the weave crash workaround on kubernetes
wget -q https://github.com/stedolan/jq/releases/download/jq-1.5/jq-linux64 && chmod +x jq-linux64 && sudo mv jq-linux64 /usr/local/bin


# install protobuf related tools
mkdir -p ~/go && cd ~/go
export GOPATH=$PWD
rm -rf protobuf
git clone --depth=1 https://github.com/google/protobuf.git
cd protobuf
./autogen.sh && ./configure && make
sudo make install && sudo ldconfig

# Install Swagger
sudo curl -o /usr/local/bin/swagger -L'#' https://github.com/go-swagger/go-swagger/releases/download/0.10.0/swagger_linux_amd64
sudo chmod +x /usr/local/bin/swagger

# Install OVS for hostsim
wget http://cbs.centos.org/kojifiles/packages/openvswitch/2.5.0/2.el7/x86_64/openvswitch-2.5.0-2.el7.x86_64.rpm -O /tmp/openvswitch-2.5.0-2.el7.x86_64.rpm
sudo yum install -y /tmp/openvswitch-2.5.0-2.el7.x86_64.rpm

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
go get  "github.com/kardianos/govendor"
go get  "github.com/gogo/protobuf/protoc-gen-gofast"
go get  "github.com/golang/protobuf/protoc-gen-go"
go get  "github.com/GeertJohan/go.rice/rice"
gometalinter -i
ENDF

chmod +x /tmp/goinstall.bash
sudo bash -c /tmp/goinstall.bash
sudo rm -fr ~/go /tmp/*

exit 0
