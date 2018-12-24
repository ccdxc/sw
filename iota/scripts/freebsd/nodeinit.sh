#! /usr/local/bin/bash
set -e

rm -f /root/.ssh/known_hosts
rm -rf /pensando
mkdir /pensando
chown vm:vm /pensando

cd /naples
tar xf drivers-freebsd.tar.xz
cd drivers-freebsd
env OS_DIR=/usr/src ./build.sh
kldload sys/modules/ionic/ionic.ko
sleep 2

ifconfig ionic2 169.254.0.2/24
ping -c 5 169.254.0.1
