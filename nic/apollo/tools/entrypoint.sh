#!/bin/sh
set -e

dir=/usr/src/github.com/pensando/sw
mkdir -p $dir
mkdir -p /sw/
mount -o bind /sw ${dir}
cd $dir/nic
exec "$@"
