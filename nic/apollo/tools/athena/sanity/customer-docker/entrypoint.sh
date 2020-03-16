#!/bin/sh

echo "spawning developer docker"
set -euo pipefail
netns=/var/run/netns

term() {
   killall dockerd
   wait
}

dockerd -s vfs &

trap term INT TERM

echo "start with entrypoint.sh"
mkdir -p ${netns}
sysctl -w vm.max_map_count=262144
#cp -R ../vendor/github.com/gogo/protobuf/proto/* /usr/local/include/
#echo 1>&2 building tools...
#cd $dir && make ws-tools && go install ./asset-build/... && make pull-assets && cd /usr/src/github.com/pensando/sw/nic
sleep 10
cd /sw
echo "start with Docker load"
echo "$@"
docker load -i customimage.tar.gz
echo "Done with Docker load"
docker run -it --rm --sysctl net.ipv6.conf.all.disable_ipv6=1 --privileged --name customer_dind customimage:v2 "$@"
echo "Done with Docker run"
exec sh -c exit 
