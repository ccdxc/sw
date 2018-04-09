from "centos:7.3.1611"


run "yum -y install https://centos7.iuscommunity.org/ius-release.rpm"
run "yum -y install python36u python36u-pip"
run "yum -y install softhsm libtool-ltdl-devel"
run "ln -s /usr/bin/python3.6 /usr/bin/python3"
run "ln -s /usr/bin/pip3.6 /usr/bin/pip3"
run "yum install -y epel-release"
run "yum install -y nfs-utils nfs-utils-lib"


PIP2_PACKAGES = %w[
  ply==3.9
  cmd2
  bitstring
  tftpy
]

PIP3_PACKAGES = %w[
  ruamel.yaml
  scapy-python3
  google-api-python-client
  enum34
  zmq
  cmd2
  bitstring
  jsonrpc2_zeromq
  setuptools
  pipenv
  pyyaml
]

PACKAGES = %w[
  bison
  python36u-devel
  python-pip
  python-devel
  python-yaml
  cmake
  git
  gcc
  gcc-c++
  gdb
  wget
  curl
  unzip
  wget
  dhcp
  dhclient
  net-tools
]

run "yum install -y #{PACKAGES.join(" ")}"

run "pip install --upgrade #{PIP2_PACKAGES.join(" ")}"
run "pip3 install --upgrade #{PIP3_PACKAGES.join(" ")}"

