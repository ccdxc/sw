from "centos:7.3.1611"

run "yum -y install https://centos7.iuscommunity.org/ius-release.rpm"
run "yum -y install python36u python36u-pip"
run "ln -s /usr/bin/python3.6 /usr/bin/python3"
run "ln -s /usr/bin/pip3.6 /usr/bin/pip3"
run "yum -y install http://li.nux.ro/download/nux/dextop/el7/x86_64/nux-dextop-release-0-5.el7.nux.noarch.rpm"


PIP2_PACKAGES = %w[
  ply==3.9
  bitstring
  tftpy
]

PIP3_PACKAGES = %w[
  ruamel.yaml
  scapy-python3
  enum34
  pyyaml
]

PACKAGES = %w[
  python36u-devel
  python-pip
  python-devel
  python-yaml
  wget
  curl
  libpcap-devel
  make
  gcc
  g++
  gdb
  tcpdump
  unzip
  wget
  dhcp
  dhclient
  net-tools
  iperf
  tftp
  ftp
  live555-tools
]

run "yum install -y #{PACKAGES.join(" ")}"

run "pip install --upgrade #{PIP2_PACKAGES.join(" ")}"
run "pip3 install --upgrade #{PIP3_PACKAGES.join(" ")}"

run "mkdir -p /iota/tools/raw"

#copy "raw.tar.gz", "/iota/tools/raw/"

