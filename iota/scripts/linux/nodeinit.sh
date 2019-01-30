#! /bin/bash
set -e

while [[ "$#" > 0 ]]; do
    case $1 in
        -c|--cleanup) cleanup=1;;
        *) echo "Unknown parameter passed: $1"; exit 1;;
    esac; shift;
done

rm -f /root/.ssh/known_hosts
chown vm:vm /pensando

dhcp_disable() {
    # Check if its centos
    os_str=`awk -F= '/^NAME/{print $2}' /etc/os-release`
    if [[ $os_str == *"Ubuntu"* ]]; then
        echo "Ubuntu: No need to disable DHCP on Naples IFs"
        return
    elif [[ $os_str == *"CentOS"* ]]; then
        echo "CentOS: Explicitly disabling DHCP on Naples IFs"
        declare -a ifs=(`systool -c net | grep "Class Device"  | tail -4 | head -3 | cut -d = -f 2 | cut -d \" -f 2`)
        for i in  "${ifs[@]}"
        do
            echo "$i"
            echo "DEVICE=$i" > /etc/sysconfig/network-scripts/ifcfg-$i
            echo "BOOTPROTO=none" >> /etc/sysconfig/network-scripts/ifcfg-$i
            echo "ONBOOT=yes" >> /etc/sysconfig/network-scripts/ifcfg-$i
        done
        sudo service network restart
        for i in  "${ifs[@]}"
        do
           ifconfig $i up
        done
    fi
}

if [ -n "$cleanup" ]; then
    rm -rf /pensando
    mkdir /pensando
    driver_dir="/naples/drivers-linux-eth"
    if [ ! -d "$driver_dir" ]; then
        echo "Cleanup Failed. No driver dir: $driver_dir"
        exit 0
    fi
    cd $driver_dir
    rmmod drivers/eth/ionic/ionic.ko 2> /dev/null || rc=$?
    if [ -n "$rc" ] && [ $rc -ne 0 ]; then
        echo "Failed to unload ionic driver. Ignore"
        exit 0
    fi
    echo "Unloaded ionic driver."
else
    cd /naples/
    tar xf drivers-linux-eth.tar.xz
    cd drivers-linux-eth
    rmmod drivers/eth/ionic/ionic.ko 2> /dev/null || rc=$?
    ./setup_apt.sh
    ./build.sh
    insmod drivers/eth/ionic/ionic.ko
    sleep 2
    intmgmt=`systool -c net | grep "Class Device"  | tail -2 | head -1 | cut -d = -f 2 | cut -d \" -f 2`
    dhcp_disable
    ifconfig $intmgmt 169.254.0.2/24
    ping -c 5 169.254.0.1
fi
