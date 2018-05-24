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
    python-paramiko python-setuptools kernel-devel glibc.i686 libgcc.i686 \
    libstdc++.i686 glibc-devel.i686 kubelet kubeadm kubectl kubernetes-cni \
    device-mapper-persistent-data lvm2 docker-ce libpcap-devel



cd /tmp
wget https://dl.google.com/go/go1.10.linux-amd64.tar.gz
sudo tar -C /usr/local -xzf go1.10.linux-amd64.tar.gz
rm go1.10.linux-amd64.tar.gz


# Install Swagger
sudo curl -o /usr/local/bin/swagger -L'#' https://github.com/go-swagger/go-swagger/releases/download/0.7.4/swagger_linux_amd64
sudo chmod +x /usr/local/bin/swagger

#jq is needed for the weave crash workaround on kubernetes
wget -q https://github.com/stedolan/jq/releases/download/jq-1.5/jq-linux64 && chmod +x jq-linux64 && sudo mv jq-linux64 /usr/local/bin

# Install Swagger
sudo curl -o /usr/local/bin/swagger -L'#' https://github.com/go-swagger/go-swagger/releases/download/0.10.0/swagger_linux_amd64
sudo chmod +x /usr/local/bin/swagger

# Install OVS for hostsim
wget http://cbs.centos.org/kojifiles/packages/openvswitch/2.5.0/2.el7/x86_64/openvswitch-2.5.0-2.el7.x86_64.rpm -O /tmp/openvswitch-2.5.0-2.el7.x86_64.rpm
sudo yum install -y /tmp/openvswitch-2.5.0-2.el7.x86_64.rpm

# install all base packages
sudo yum -y install https://centos7.iuscommunity.org/ius-release.rpm
sudo yum -y install python36u python36u-pip
sudo ln -s /usr/bin/python3.6 /usr/bin/python3
sudo ln -s /usr/bin/pip3.6 /usr/bin/pip3
sudo yum install -y vim bison python36u-devel python-pip python-devel \
         python-yaml Judy-devel cmake git git gcc-c++ gcc-c++ wget autoconf \
         automake libtool libtool make g++ unzip pkg-config graphviz flex \
         gmp-devel mpfr-devel mpc-devel libmpc-devel bzip2 which \
         python-setuptools python-enum34 swig openssl-devel libpcap-devel \
         cscope ctags softhsm libtool-ltdl-devel nfs-utils nfs-utils-lib dhcp \
         dhclient kernel-devel kernel-headers dkms iproute2 net-tools zip \
         zlib1g-dev gdb

sudo yum install -y http://dl.fedoraproject.org/pub/epel/7/x86_64/Packages/j/Judy-1.0.5-8.el7.x86_64.rpm
sudo yum install -y http://dl.fedoraproject.org/pub/epel/7/x86_64/Packages/j/Judy-devel-1.0.5-8.el7.x86_64.rpm

# install all python3 packages
sudo pip3 install --upgrade ruamel.yaml scapy-python3 google-api-python-client Tenjin enum34 protobuf grpcio zmq cmd2 PyOpenSSL pysendfile pyftpdlib jsonrpc2_zeromq
sudo pip3 install tenjin
sudo pip3 install click click_repl
sudo pip3 install docker

# install all python2.7 packages
curl "https://bootstrap.pypa.io/get-pip.py" -o "/tmp/get-pip.py"
sudo python /tmp/get-pip.py
sudo pip install --upgrade pip
#sudo pip install --upgrade ply==3.9 cmd2 tftpy
sudo pip install --upgrade cmd2 tftpy

#install pipenv
sudo pip install setuptools
sudo pip install --upgrade setuptools
sudo pip install pipenv

# install protobuf related tools
mkdir -p ~/go && cd ~/go
export GOPATH=$PWD
rm -rf protobuf
git clone --depth=1 https://github.com/google/protobuf.git
cd protobuf
./autogen.sh && ./configure && make
sudo make install && sudo ldconfig

# install gcc 6.1
cd /tmp
curl ftp://ftp.gnu.org/pub/gnu/gcc/gcc-6.1.0/gcc-6.1.0.tar.bz2 | tar xj
cd gcc-6.1.0
./configure --enable-languages=c,c++ --disable-multilib --prefix=/usr
make -j$(grep -c processor /proc/cpuinfo)
sudo make install
sudo ldconfig

# install zeromq
cd /tmp
wget https://github.com/zeromq/zeromq3-x/releases/download/v3.2.5/zeromq-3.2.5.tar.gz
tar zxf zeromq-3.2.5.tar.gz
cd zeromq-3.2.5
./configure --prefix=/usr
make -j$(grep -c processor /proc/cpuinfo)
sudo make install

# install libtins
cd /tmp
git clone https://github.com/mfontanini/libtins.git
cd libtins
mkdir build
cd build
cmake ../ -DLIBTINS_ENABLE_CXX11=1
make
sudo make install
sudo ldconfig

# install bazel
cd /tmp
wget https://github.com/bazelbuild/bazel/releases/download/0.5.4/bazel-0.5.4-installer-linux-x86_64.sh
chmod +x bazel-0.5.4-installer-linux-x86_64.sh
sudo ./bazel-0.5.4-installer-linux-x86_64.sh
sudo sh -c "echo /usr/local/lib >>/etc/ld.so.conf"
sudo ldconfig

# install valgrind
cd /tmp
mkdir valgrind
cd valgrind
wget ftp://sourceware.org/pub/valgrind/valgrind-3.11.0.tar.bz2
tar xvjf valgrind-3.11.0.tar.bz2
cd valgrind-3.11.0
./autogen.sh
./configure
make
sudo make install

# cleanup tmp folder
sudo rm -fr ~/go /tmp/*

exit 0
