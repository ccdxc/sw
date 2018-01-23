#!/bin/sh

set -euo pipefail

dir=/usr/src/github.com/pensando/sw

mkdir -p ${dir}
mount -o bind /sw ${dir}
echo 1>&2 building asset tools...
mkdir -p /tool
mount asic20.pensando.io:/export/tool /tool

cp -R ../vendor/github.com/gogo/protobuf/proto/* /usr/local/include/

cd $dir && go install ./asset-build/... && cd /sw/dol

exec "$@"

exec "$@"
