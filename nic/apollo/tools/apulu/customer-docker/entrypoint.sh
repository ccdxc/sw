#!/bin/sh

echo "spawning developer docker"
set -euo pipefail
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
cp -R ../vendor/github.com/gogo/protobuf/proto/* /usr/local/include/
echo 1>&2 building tools...
cd $dir && make ws-tools && go install ./asset-build/... && make pull-assets && cd /usr/src/github.com/pensando/sw/nic

cd /sw/nic

# do this precomilation here in the developer workspace before forking the
# container. Else some .so files can't be copied from /sw/nic to
# /sw/apollo_sw/nic
./apollo/tools/build_custom_docker.sh clean apulu
make PIPELINE=apulu
make PIPELINE=apulu ARCH=aarch64 PLATFORM=hw
make PIPELINE=apulu ARCH=aarch64 PLATFORM=hw firmware

echo "starting dev-docker creation for apulu"
./apollo/tools/dev-docker/build_dev_docker.sh 1 all apulu

cd /
if [ -d /sw/apollo_sw ];then
    mount --bind /sw/apollo_sw /sw
    mount -o bind /sw ${dir}
fi

cd /sw/nic

./apollo/tools/build_custom_docker.sh aarch64 apulu
./apollo/tools/build_custom_docker.sh firmware apulu
./apollo/tools/build_custom_docker.sh clean apulu
./apollo/tools/build_custom_docker.sh x86_64 apulu

exec "$@"
