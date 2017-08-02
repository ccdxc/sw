from "centos:7.3.1611"

ROOT = "/hack/saratk/nic"
BASE_BUILD_DIR = "/tmp/build"

env GOPATH: "/go",
    PATH: "/usr/local/bin:/bin:/sbin:/usr/bin:/usr/sbin:/usr/local/go/bin:/go/bin",
    PKG_CONFIG_PATH: "/usr/local/lib/pkgconfig"

run "yum -y install https://centos7.iuscommunity.org/ius-release.rpm"
run "yum -y install python36u python36u-pip"
run "ln -s /usr/bin/python3.6 /usr/bin/python3"
run "ln -s /usr/bin/pip3.6 /usr/bin/pip3"

PIP2_PACKAGES = %w[
  ply==3.9
]

PIP3_PACKAGES = %w[
  ruamel.yaml
  scapy-python3
  google-api-python-client
  Tenjin
  enum34
]

PACKAGES = %w[
  bison
  python36u-devel
  python-pip
  python-devel
  Judy-devel
  cmake
  git
  gcc
  gcc-c++
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
]

run "yum install -y #{PACKAGES.join(" ")}"

# otherwise protobuf.pc will end up in the wrong spot, required for building protobuf-c
run "ln -s /usr/share/pkgconfig /usr/lib/pkgconfig"

run "curl -sSL https://storage.googleapis.com/golang/go1.8.3.linux-amd64.tar.gz | tar xz -C /usr/local"
run "go get github.com/golang/protobuf/... google.golang.org/grpc"

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
  run "wget https://github.com/google/protobuf/releases/download/v3.3.0/protobuf-cpp-3.3.0.tar.gz"
  run "tar xzf protobuf-cpp-3.3.0.tar.gz"
end

# install protobuf-cpp
inside "#{BASE_BUILD_DIR}/protobuf-3.3.0" do
  run "./autogen.sh && ./configure --prefix /usr"
  run "make -j$(grep -c processor /proc/cpuinfo)"
  run "make install"
  run "ldconfig"
end

# prep protobuf-c
inside BASE_BUILD_DIR do
  run "wget https://github.com/protobuf-c/protobuf-c/releases/download/v1.2.1/protobuf-c-1.2.1.tar.gz"
  run "tar -xzf protobuf-c-1.2.1.tar.gz"
end

# install protobuf-c
inside "#{BASE_BUILD_DIR}/protobuf-c-1.2.1" do
  run "./configure --prefix /usr"
  run "make -j$(grep -c processor /proc/cpuinfo)"
  run "make install"
  run "ldconfig"
end

# prep grpc
inside BASE_BUILD_DIR do
  run "git clone -b $(curl -L http://grpc.io/release) https://github.com/grpc/grpc"
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

# prep zeromq
inside BASE_BUILD_DIR do
  run "wget https://github.com/zeromq/zeromq3-x/releases/download/v3.2.5/zeromq-3.2.5.tar.gz"
  run "tar zxf zeromq-3.2.5.tar.gz"
end

# install zeromq
inside "#{BASE_BUILD_DIR}/zeromq-3.2.5" do
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

run "pip3 install --upgrade #{PIP3_PACKAGES.join(" ")}"
run "pip install --upgrade #{PIP2_PACKAGES.join(" ")}"

copy "sknobs.tar.gz", "#{BASE_BUILD_DIR}/sknobs.tar.gz"
inside "/" do
  run "tar xzf #{BASE_BUILD_DIR}/sknobs.tar.gz"
end

run "mkdir -p #{ROOT}"
workdir ROOT

entrypoint []
cmd "bash"

tag "pensando/nic:dependencies"

run "rm -rf #{BASE_BUILD_DIR}" # this has no effect on size until the flatten is processed

if getenv("RELEASE") != ""
  flatten
end

tag "srv1.pensando.io:5000/pensando/nic:dependencies"
