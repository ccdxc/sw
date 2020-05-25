#!/bin/sh

set -euo pipefail
BOX_START_DIR=$(pwd)
BOX_SUB_DIR=$(pwd | cut -d '/' -f 3-)
echo "Box start dir=$BOX_START_DIR, sub_dir=$BOX_SUB_DIR"
dir=/usr/src/github.com/pensando/sw
netns=/var/run/netns

term() {
	killall dockerd
	wait
}

dockerd -s vfs &

trap term INT TERM

mkdir -p ${dir}
mkdir -p ${netns}
mount -o bind /sw ${dir}
sysctl -w vm.max_map_count=262144
export GOPATH="/usr"
cp -R $dir/vendor/github.com/gogo/protobuf/proto/* /usr/local/include/
echo 1>&2 building tools...
cd $dir && make ws-tools && go install ./asset-build/... && make pull-assets
echo "cd $dir/$BOX_SUB_DIR"
cd $dir/$BOX_SUB_DIR
pwd

exec "$@"
