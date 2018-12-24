#! /bin/bash
set -e

rm -f /root/.ssh/known_hosts
rm -rf /pensando
mkdir /pensando
chown vm:vm /pensando

cd /naples/
tar xf drivers-linux.tar.xz
cd drivers-linux
./setup_apt.sh
./build.sh
insmod drivers/eth/ionic/ionic.ko
sleep 2

intmgmt=`systool -c net | grep "Class Device"  | tail -2 | head -1 | cut -d = -f 2 | cut -d \" -f 2`
ifconfig $intmgmt 169.254.0.2/24
ping -c 5 169.254.0.1 
