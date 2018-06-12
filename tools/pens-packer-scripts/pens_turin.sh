#!/bin/bash

set -e
set -x

if [ "$PACKER_BUILDER_TYPE" != "virtualbox-iso" ]; then
  exit 0
fi

echo "*************** TURIN BUILD *****************"
# Install and start docker
mkdir install; cd install
curl -o docker.rpm https://download.docker.com/linux/centos/7/x86_64/stable/Packages/docker-ce-18.03.1.ce-1.el7.centos.x86_64.rpm
sudo yum install -y docker.rpm
sudo systemctl start docker
sudo systemctl enable docker
sudo rm -rf docker.rpm

# install ovs
curl -o ovs.rpm http://cbs.centos.org/kojifiles/packages/openvswitch/2.9.0/4.el7/x86_64/openvswitch-2.9.0-4.el7.x86_64.rpm
sudo yum install -y ovs.rpm
sudo systemctl start openvswitch.service
sudo systemctl enable openvswitch
sudo rm -rf ovs.rpm

# install tools
# sudo yum install -y https://dl.fedoraproject.org/pub/epel/epel-release-latest-7.noarch.rpm
curl --silent --location https://rpm.nodesource.com/setup_8.x | sudo bash -
sudo yum install -y bridge-utils tcpdump net-tools nodejs npm nc
sudo npm install -g newman
