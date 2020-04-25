from "centos:7.3.1611"

ROOT = "/sw"
BASE_BUILD_DIR = "/tmp/build"

env GOPATH: "/go",
    PATH: "/usr/local/bin:/bin:/sbin:/usr/bin:/usr/sbin:/usr/local/go/bin:/go/bin",
    PKG_CONFIG_PATH: "/usr/local/lib/pkgconfig"

run "yum -y install https://centos7.iuscommunity.org/ius-release.rpm"
run "yum -y install python36u python36u-pip"
run "yum -y install softhsm libtool-ltdl-devel"
run "ln -sf /usr/bin/python3.6 /usr/bin/python3"
run "ln -sf /usr/bin/pip3.6 /usr/bin/pip3"
run "yum install -y epel-release"
run "yum install -y epel-release.noarch bash-completion.noarch" # For halctl bash-completion
run "yum install -y centos-release-scl" # Needed by buildroot for newer "make" installed by devtoolset-7
run "yum install -y numactl-devel libuuid-devel libaio-devel CUnit-devel" # For storage/offload
run "yum install -y patch libedit2 libedit-devel" # For platform

PIP2_PACKAGES = %w[
  ply==3.9
  cmd2
  bitstring
  tftpy
  pexpect
]

PIP3_PACKAGES = %w[
  ruamel.yaml==0.15.80
  scapy-python3
  google-api-python-client
  Tenjin
  enum34
  protobuf
  grpcio
  zmq
  cmd2
  bitstring
  PyOpenSSL
  pysendfile
  pyftpdlib
  jsonrpc2_zeromq
  setuptools
  pipenv
  pyyaml
  docker
  pexpect
  paramiko
]

PACKAGES = %w[
  bc
  bison
  perl-ExtUtils-MakeMaker
  python36u-devel
  python-pip
  python-devel
  python-yaml
  Judy-devel
  cmake
  git
  gcc
  gcc-c++
  gdb
  wget
  autoconf
  automake
  libtool
  curl
  make
  g++
  unzip
  pkg-config
  graphviz
  flex
  gmp-devel
  mpfr-devel
  mpc-devel
  libmpc-devel
  bzip2
  which
  python-setuptools
  python-enum34
  swig
  openssl-devel
  libpcap-devel
  pcre-devel
  luajit-devel
  hwloc-devel
  iptables-devel
  libdnet-devel
  zlib-devel
  iproute
  dhcp
  dhclient
  libedit-devel
  patch
  glib2-devel
  nmap
  libcap-devel
  sudo
  telnet
  sshpass
  ipmitool
  libxml2-devel
  devtoolset-7-make.x86_64
  jq
  perl-Archive-Zip
  tcpdump
  qemu-img
  clang
  nfs-utils
  nfs-utils-lib
]

run "yum install -y #{PACKAGES.join(" ")}"

# otherwise protobuf.pc will end up in the wrong spot, required for building protobuf-c
run "ln -s /usr/share/pkgconfig /usr/lib/pkgconfig"

# Install go 1.13.4
run "curl -sSL https://dl.google.com/go/go1.13.4.linux-amd64.tar.gz | tar xz -C /usr/local"
run "go get github.com/golang/protobuf/..."

run "yum install epel-release"
run "rpm -Uvh https://mirror.webtatic.com/yum/el6/latest.rpm"
# install docker. This is needed for e2e
run "curl -sSL https://get.docker.com | CHANNEL=stable bash"
copy "tools/test-build/daemon.json", "/etc/docker/daemon.json"

inside BASE_BUILD_DIR do
  run "curl ftp://ftp.gnu.org/pub/gnu/gcc/gcc-6.1.0/gcc-6.1.0.tar.bz2 | tar xj"
end

inside "#{BASE_BUILD_DIR}/gcc-6.1.0" do
  run "./configure --enable-languages=c,c++ --disable-multilib --prefix=/usr"
  run "make -j$(grep -c processor /proc/cpuinfo)"
  run "make install"
  run "ldconfig"
end

# prep protobuf-cpp
inside BASE_BUILD_DIR do
  run "wget https://github.com/google/protobuf/releases/download/v3.4.1/protobuf-cpp-3.4.1.tar.gz"
  run "tar xzf protobuf-cpp-3.4.1.tar.gz"
end

# install protobuf-cpp
inside "#{BASE_BUILD_DIR}/protobuf-3.4.1" do
  run "./autogen.sh && ./configure --prefix /usr"
  run "make -j$(grep -c processor /proc/cpuinfo)"
  run "make install"
  run "ldconfig"
end

# prep protobuf-c
inside BASE_BUILD_DIR do
  run "wget https://github.com/protobuf-c/protobuf-c/releases/download/v1.3.0/protobuf-c-1.3.0.tar.gz"
  run "tar -xzf protobuf-c-1.3.0.tar.gz"
end

# install protobuf-c
inside "#{BASE_BUILD_DIR}/protobuf-c-1.3.0" do
  run "./configure --prefix /usr"
  run "make -j$(grep -c processor /proc/cpuinfo)"
  run "make install"
  run "ldconfig"
end

# prep grpc
inside BASE_BUILD_DIR do
  run "git clone -b v1.6.6 https://github.com/grpc/grpc"
end

# install grpc
# installs into /usr/local prefix instead of the /usr convention. Not sure how
# to do this with cmake.
inside "#{BASE_BUILD_DIR}/grpc" do
  run "git submodule update --init"
  run "make -j$(grep -c processor /proc/cpuinfo)"
  run "make install"
  run "ldconfig"
end

# prep valgrind
inside BASE_BUILD_DIR do
  run "mkdir valgrind"
end

# prep valgrind
inside "#{BASE_BUILD_DIR}/valgrind" do
  run "wget ftp://sourceware.org/pub/valgrind/valgrind-3.11.0.tar.bz2"
  run "tar xvjf valgrind-3.11.0.tar.bz2"
end

# install valgrind
inside "#{BASE_BUILD_DIR}/valgrind/valgrind-3.11.0" do
  run "./autogen.sh"
  run "./configure"
  run "make"
  run "make install"
end

# prep zeromq
inside BASE_BUILD_DIR do
  run "wget https://github.com/zeromq/zeromq4-x/releases/download/v4.0.0/zeromq-4.0.0-rc1.tar.gz"
  run "tar zxf zeromq-4.0.0-rc1.tar.gz"
end

# install zeromq
inside "#{BASE_BUILD_DIR}/zeromq-4.0.0" do
  run "./configure --prefix=/usr"
  run "make -j$(grep -c processor /proc/cpuinfo)"
  run "make install"
end

inside BASE_BUILD_DIR do
  run "wget http://downloads.sourceforge.net/project/boost/boost/1.62.0/boost_1_62_0.tar.gz"
  run "tar xfz boost_1_62_0.tar.gz"
end

inside "#{BASE_BUILD_DIR}/boost_1_62_0" do
  run "./bootstrap.sh"
  run "./b2 --without-python --prefix=/usr -j$(grep -c processor /proc/cpuinfo) link=shared runtime-link=shared install"
  run "ldconfig"
end

inside BASE_BUILD_DIR do
  run "git clone https://github.com/mfontanini/libtins.git"
end

inside "#{BASE_BUILD_DIR}/libtins" do
  run "mkdir build"
end

inside "#{BASE_BUILD_DIR}/libtins/build" do
  run "cmake ../ -DLIBTINS_ENABLE_CXX11=1"
  run "make"
  run "make install"
  run "ldconfig"
end

inside BASE_BUILD_DIR do
  run "git clone https://gitlab.isc.org/isc-projects/bind9.git"
end

inside "#{BASE_BUILD_DIR}/bind9" do
  run "git checkout v9_10"
  run "./configure "
  run "make"
  run "make install"
end

#run "yum install -y zeromq-devel"

run "pip install --upgrade pip"
run "pip install --ignore-installed --upgrade #{PIP2_PACKAGES.join(" ")}"
run "pip3 install --upgrade #{PIP3_PACKAGES.join(" ")}"

run "yum install -y dkms iproute2 net-tools zip zlib1g-dev"
inside BASE_BUILD_DIR do
  run "wget https://github.com/bazelbuild/bazel/releases/download/0.14.1/bazel-0.14.1-installer-linux-x86_64.sh"
  run "chmod +x bazel-0.14.1-installer-linux-x86_64.sh"
  run "./bazel-0.14.1-installer-linux-x86_64.sh"
end

copy "nic/toolchain.tar.gz", "#{BASE_BUILD_DIR}/toolchain.tar.gz"
inside "/" do
  run "tar xzf #{BASE_BUILD_DIR}/toolchain.tar.gz"
end

DAQ_VERSION = "2.2.2"
inside BASE_BUILD_DIR do
  run "wget https://www.snort.org/downloads/snortplus/daq-#{DAQ_VERSION}.tar.gz \
       && tar xvfz daq-#{DAQ_VERSION}.tar.gz \
       && cd daq-#{DAQ_VERSION} \
       && CFLAGS=\"$CFLAGS -fPIC\" ./configure && make && make install"
end

OPENAPP_VERSION = "12159"
inside BASE_BUILD_DIR do
  run "wget https://www.snort.org/downloads/openappid/#{OPENAPP_VERSION} \
       && mv #{OPENAPP_VERSION} snort-openappid.tar.gz \
       && tar -zxvf snort-openappid.tar.gz"
end

COREUTILS_VERSION="8.30"
inside BASE_BUILD_DIR do
  run "wget https://ftp.gnu.org/gnu/coreutils/coreutils-#{COREUTILS_VERSION}.tar.xz \
       && xz -d coreutils-#{COREUTILS_VERSION}.tar.xz \
       && tar xvf coreutils-#{COREUTILS_VERSION}.tar \
       && cd coreutils-#{COREUTILS_VERSION} \
       && FORCE_UNSAFE_CONFIGURE=1 ./configure --prefix=/opt/coreutils \
       && make \
       && make install"
end

run "mkdir -p /var/log/snort && \
     mkdir -p /usr/local/lib/snort_dynamicrules && \
     mkdir -p /etc/snort && \
     mkdir -p /etc/snort/rules && \
     mkdir -p /etc/snort/preproc_rules && \
     mkdir -p /etc/snort/so_rules && \
     mkdir -p /etc/snort/etc
     cp -r #{BASE_BUILD_DIR}/odp /etc/snort/odp"

run "yum install -y lcov"

run "mkdir -p /opt/trex && cd /opt/trex && \
	wget --no-check-certificate --no-cache https://trex-tgn.cisco.com/trex/release/v2.65.tar.gz && tar -xzvf v2.65.tar.gz"

run "yum install -y wireshark"

inside BASE_BUILD_DIR do
  run "git clone https://github.com/secdev/scapy && cd scapy && python3 setup.py install"
end

run "mkdir -p #{ROOT}"
workdir "/sw/nic"

entrypoint []
cmd "bash"

tag "pensando/nic:1.42"

run "rm -rf #{BASE_BUILD_DIR}" # this has no effect on size until the flatten is processed

run "echo /usr/local/lib >>/etc/ld.so.conf"
run "ldconfig -v"

run "yum clean all"

after do
  if getenv("RELEASE") != ""
    flatten
  end
end
