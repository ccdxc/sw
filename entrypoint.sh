#!/bin/sh

set -euo pipefail
dir=/usr/src/github.com/pensando/sw
netns=/var/run/netns

mkdir -p ${dir}
mkdir -p ${netns}
mount -o bind /sw ${dir}
sysctl -w vm.max_map_count=262144

export GOPATH="/usr"

term() {
	killall dockerd
	wait
}

dockerd -s vfs &

trap term INT TERM

cp -R ./vendor/github.com/gogo/protobuf/proto/* /usr/local/include/

exec "$@"
