#!/bin/sh
set -e
echo "Entered here"
echo "$@"
#dir=/usr/src/github.com/pensando/sw
#mkdir -p $dir
#mkdir -p /sw/
#mount -o bind  ${dir} /sw
#cd $dir/nic
export GOPATH='/usr'
export PATH=$PATH:/usr/local/go/bin
cd /sw/nic
#echo "Invoke build_custom_docker with aarch64/athena"
#./apollo/tools/build_custom_docker.sh aarch64 athena
#echo "Invoke build_custom_docker with firmware/athena"
#./apollo/tools/build_custom_docker.sh firmware athena
echo "Invoke build_custom_docker with x86_64/athena"
./apollo/tools/build_custom_docker.sh x86_64 athena

echo "End of script"
pwd
    
exec "$@"
