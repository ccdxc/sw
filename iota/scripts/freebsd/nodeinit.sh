#! /usr/local/bin/bash
set -e

own_ip="169.254.0.2"
trg_ip="169.254.0.1"

while [[ "$#" > 0 ]]; do
    case $1 in
        -c|--cleanup) cleanup=1;;
        --skip-install) skip_install=1;;
        --own_ip) own_ip=$2; shift;;
        --trg_ip) trg_ip=$2; shift;;
        *) echo "Unknown parameter passed: $1"; exit 1;;
    esac; shift;
done

rm -f /root/.ssh/known_hosts
chown vm:vm /pensando

if [ -n "$skip_install" ]; then
    echo "user requested to skip install"
else
    if [ -n "$cleanup" ]; then
        rm -rf /pensando
        mkdir /pensando
        driver_dir="/naples/drivers-freebsd-eth"
        if [ ! -d "$driver_dir" ]; then
            echo "Cleanup Failed. No driver dir: $driver_dir"
            exit 0
        fi
        cd $driver_dir
        kldunload sys/modules/ionic/ionic.ko 2> /dev/null || rc=$?
        if [ -n "$rc" ] && [ $rc -ne 0 ]; then
            echo "Failed to unload ionic driver. Ignore"
            exit 0
        fi
        echo "Unloaded ionic driver."
    else

        # For debugging INFRA-122
        echo '---'
        echo 'Installed kernel images:'
        ls -ld /boot/kernel*/kernel
        echo 'Currently running kernel:'
        uname -a
        echo 'Log from PXE setup script:'
        cat /tmp/log.txt
        echo '---'

        cd /naples
        tar xf drivers-freebsd-eth.tar.xz
        cd drivers-freebsd-eth
        env OS_DIR=/usr/src ./build.sh
        kldunload sys/modules/ionic/ionic.ko 2> /dev/null || rc=$?
        kldload sys/modules/ionic/ionic.ko || (dmesg && exit 1)
        sleep 2
        intmgmt=`pciconf -l | grep chip=0x10041dd8 | cut -d'@' -f1 | sed "s/ion/ionic/g"`
        ifconfig $intmgmt $own_ip/24
        if [ -n "$no_mgmt" ]; then
            echo "Skip ping test of internal mgmt interface on host"
            exit 0
        fi
        if ! (ping -c 5 $trg_ip); then
            ./print-cores.sh
            exit 1
        fi
    fi
fi

