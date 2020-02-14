#! /bin/bash
set -e

own_ip="169.254.XX.2"
trg_ip="169.254.XX.1"

while [[ "$#" > 0 ]]; do
    case $1 in
        -c|--cleanup) cleanup=1;;
        --no-mgmt) no_mgmt=1;;
        --skip-install) skip_install=1;;
        --own_ip) own_ip=$2; shift;;
        --trg_ip) trg_ip=$2; shift;;
        *) echo "Unknown parameter passed: $1"; exit 1;;
    esac; shift;
done

rm -f /root/.ssh/known_hosts
chown vm:vm /pensando

# find all the ionic interfaces and the mgmt interfaces
ifs=""
mgmt_ifs=""
for net in /sys/class/net/* ; do
    n=`basename $net`
    if [ ! -e $net/device/vendor ] ; then
    continue
    fi
    v=`cat $net/device/vendor`
    if [ $v != "0x1dd8" ] ; then
        continue
    fi

    ifs+=" $n"
    d=`cat $net/device/device`
    if [ $d == "0x1004" ] ; then
        mgmt_ifs+=" $n"
    fi
done

dhcp_disable() {
    # Check if it is centos
    os_str=`awk -F= '/^NAME/{print $2}' /etc/os-release`
    os_version=`awk -F= '$1=="VERSION_ID" { print $2 ;}' /etc/os-release | sed -e 's/\([678]\)\../\1/'`
    if [[ $os_str == *"Ubuntu"* ]]; then
        echo "Ubuntu: No need to disable DHCP on Naples IFs"
        return
    elif [[ $os_str == *"CentOS"* || $os_str == *"Red Hat"* ]]; then
        echo "CentOS/RHEL: Explicitly disabling DHCP on Naples IFs"
        # Create network interface scripts
        for i in $ifs
        do
            echo "$i"
            echo "DEVICE=$i" > /etc/sysconfig/network-scripts/ifcfg-$i
            echo "BOOTPROTO=none" >> /etc/sysconfig/network-scripts/ifcfg-$i
            echo "ONBOOT=yes" >> /etc/sysconfig/network-scripts/ifcfg-$i
            echo "IPV6INIT=yes" >> /etc/sysconfig/network-scripts/ifcfg-$i
            echo "IPV6ADDR=IPv6-IP-Address" >> /etc/sysconfig/network-scripts/ifcfg-$i
            echo "IPV6_DEFAULTGW=IPv6-IP-Gateway-Address" >> /etc/sysconfig/network-scripts/ifcfg-$i
        done
        # Global enable IPV6
        echo "NETWORKING_IPV6=yes" >> /etc/sysconfig/network
        # Restart network cervices, RHEL 8 and RHEL 7 do it differently. 
        if [[ $os_version == *"8"* ]]; then
            nmcli connection load /etc/sysconfig/network-scripts/ifcfg-$i
        else
        sudo service network restart
        fi
        # Bringup all interfaces
        for i in $ifs
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
        rmmod ionic 2> /dev/null || rc=$?
        if [ -n "$rc" ] && [ $rc -ne 0 ]; then
            echo "Failed to unload ionic driver. Ignore"
            exit 0
        fi
        echo "Unloaded ionic driver."
    else
        rmmod ionic 2> /dev/null || rc=$?
        cd /naples/
        tar xf drivers-linux-eth.tar.xz
        cd drivers-linux-eth
        ./setup_libs.sh
        ./build.sh
        insmod drivers/eth/ionic/ionic.ko || (dmesg && exit 1)
        sleep 2

        dhcp_disable

        if [ -n "$no_mgmt" ]; then
            echo "Internal mgmt interface is not required."
            exit 0
        fi

        num_nic=`echo $ifs | wc -w`
        num_mgmt=`echo $mgmt_ifs | wc -w`
        num_mgmt_expected=`expr $num_nic / 3`

        if [[ $num_mgmt -ne $num_mgmt_expected ]]; then
            echo "ERROR: Internal mgmt interface is required."
            echo "Should have $num_mgmt_expected but see $num_mgmt"
            exit 1
        fi

        # override possible command line setting
        # with default settings if multiple NICs
        if [ $num_mgmt -gt 1 ] ; then
            own_ip="169.254.XX.2"
            trg_ip="169.254.XX.1"
        fi

        for intmgmt in $mgmt_ifs ; do
            pci=`ethtool -i $intmgmt | awk '/bus-info/ { print $2 }'`
            echo "Internal mgmt interface $intmgmt detected at $pci"

            s_hex=`echo $pci | cut -d: -f2 | tr [a-f] [A-F]`
            s_dec=`echo "obase=10; ibase=16; $s_hex" | bc`
            mgmt_ip=`echo $own_ip | sed -e s/XX/$s_dec/`
            mnic_ip=`echo $trg_ip | sed -e s/XX/$s_dec/`

            ifconfig $intmgmt $mgmt_ip/24
            if [ -n "$no_mgmt" ]; then
                echo "Skip ping test of internal mgmt interface on host"
                exit 0
            fi
            if ! (ping -c 5 $mnic_ip); then
                ./print-cores.sh
                exit 1
            fi
        done
    fi
fi

