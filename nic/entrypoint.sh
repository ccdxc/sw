#!/bin/sh

set -euo pipefail

dir=/usr/src/github.com/pensando/sw

mkdir -p ${dir}
mount -o bind /sw ${dir}
echo 1>&2 building asset tools...

cp -R ../vendor/github.com/gogo/protobuf/protobuf/* /usr/local/include/

cd $dir && go install ./asset-build/... && cd /sw/nic

exec "$@"
