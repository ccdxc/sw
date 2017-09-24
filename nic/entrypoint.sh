#!/bin/sh

set -euo pipefail

dir=/usr/src/github.com/pensando/sw

mkdir -p ${dir}
mount -o bind /sw ${dir}
echo 1>&2 building asset tools...

cd $dir && go install ./build/... && cd /sw/nic

exec "$@"
