#! /bin/bash
set -e

while [[ "$#" > 0 ]]; do
    case $1 in
        -c|--cleanup) cleanup=1;;
        --no-mgmt) no_mgmt=1;;
        --skip-install) skip_install=1;;
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

if [ -n "$skip_install" ]; then
    echo "user requested to skip install"
else
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
        ./setup_libs.sh
        ./build.sh
        insmod drivers/eth/ionic/ionic.ko
        sleep 2

        #intmgmt=`systool -c net | grep "Class Device"  | tail -4 | head -1 | cut -d = -f 2 | cut -d \" -f 2`
        bdf=`lspci -d :1004 | cut -d' ' -f1`

        if [ -z "$bdf" ]; then
            echo "No internal mgmt interface detected."
            if [ -n "$no_mgmt" ]; then
                echo "Internal mgmt interface is not required."
                exit 0
            fi

            echo "ERROR: Internal mgmt interface is required."
            exit 1
        fi

        intmgmt=`ls /sys/bus/pci/devices/0000:$bdf/net/`
        echo "Internal mgmt interface $intmgmt detected at $bdf."

        dhcp_disable
        ifconfig $intmgmt 169.254.0.2/24
        if [ -n "$no_mgmt" ]; then
            echo "Skip ping test of internal mgmt interface on host"
            exit 0
        fi
        if ! (ping -c 5 169.254.0.1); then
            ./print-cores.sh
            exit 1
        fi
    fi
fi

