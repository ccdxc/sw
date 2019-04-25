#!/bin/sh
set -e

dir=/usr/src/github.com/pensando/sw
mkdir -p $dir
mkdir -p /sw/
mount -o bind  ${dir} /sw
cd $dir/nic
export GOPATH='/usr'
export PATH=$PATH:/usr/local/go/bin
exec "$@"
