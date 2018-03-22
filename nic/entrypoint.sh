#!/bin/sh

set -euo pipefail
dir=/usr/src/github.com/pensando/sw

term() {
	killall dockerd
	wait
}

dockerd -s vfs &

trap term INT TERM

mkdir -p ${dir}
mount -o bind /sw ${dir}
cp -R ../vendor/github.com/gogo/protobuf/proto/* /usr/local/include/
echo 1>&2 building tools...
cd $dir && make ws-tools && go install ./asset-build/... && cd /sw/nic

exec "$@"
