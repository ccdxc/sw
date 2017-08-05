#!/bin/bash

# Install protoc compiler and protoc-gen-go extension

die () {
  echo "$@" >&2
  exit 1
}

if [ $GOPATH = "" ]; then
    die "GOPATH env variable is not set"
fi

# Install protoc
cd $GOPATH/src/github.com/google
rm -rf protobuf
git clone https://github.com/google/protobuf.git
cd protobuf
./autogen.sh && ./configure && make
sudo make install && sudo ldconfig

# Install protoc-gen-go
go get -u google.golang.org/grpc
go get -u github.com/golang/protobuf/protoc-gen-go
