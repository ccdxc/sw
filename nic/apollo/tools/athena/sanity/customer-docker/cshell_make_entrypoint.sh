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
./apollo/tools/build_custom_docker.sh clean athena
echo "Invoke regular x86 build"
make PIPELINE=athena
echo "Invoke regular aarch64 build - no firmware"
make PIPELINE=athena ARCH=aarch64 PLATFORM=hw
echo "Invoke regular aarch64 build with firmware"
make PIPELINE=athena ARCH=aarch64 PLATFORM=hw firmware

echo "Starting dev-docker creation for athena"
./apollo/tools/dev-docker/build_dev_docker.sh 0 all athena

cd /
if [ -d /sw/apollo_sw ];then
    mount --bind /sw/apollo_sw /sw
    mount -o bind /sw ${dir}
fi

cd /sw/nic

echo "Invoke build_custom_docker with aarch64/athena"
./apollo/tools/build_custom_docker.sh aarch64 athena
echo "Invoke build_custom_docker with firmware/athena"
./apollo/tools/build_custom_docker.sh firmware athena
echo "Invoke build_custom_docker with clean/athena"
./apollo/tools/build_custom_docker.sh clean athena
echo "Invoke build_custom_docker with x86_64/athena"
./apollo/tools/build_custom_docker.sh x86_64 athena
echo "Post build_custom_dockers"

exec "$@"
