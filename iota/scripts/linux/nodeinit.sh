#! /bin/bash
set -e
set -x

own_ip="169.254.XX.2"
trg_ip="169.254.XX.1"

DEFAULT_IONIC_DRIVER_PATH=/naples/drivers-linux-eth/drivers/eth/ionic/ionic.ko

while [[ "$#" > 0 ]]; do
    case $1 in
        -c|--cleanup) cleanup=1;;
        --mgmt_only) mgmt_only=1;;
        --no-mgmt) no_mgmt=1;;
        --skip-install) skip_install=1;;
        --version) version_only=1;;
        --own_ip) own_ip=$2; shift;;
        --trg_ip) trg_ip=$2; shift;;
        *) echo "Unknown parameter passed: $1"; exit 1;;
    esac; shift;
done

rm -f /root/.ssh/known_hosts
chown vm:vm /pensando
ifs=""
mgmt_ifs=""
fw_version=""

# find all the ionic interfaces and the mgmt interfaces
function init_host() {
    if [[ -f /etc/docker/daemon.json ]] ;
    then
        [ -s /etc/docker/daemon.json ] || (echo "{\"insecure-registries\" : [\"registry.test.pensando.io:5000\"]}" >> /etc/docker/daemon.json && systemctl restart docker)
    fi
    cnt=`lsmod | grep ionic | wc -l`
    if [[ $cnt -eq 0 ]] ; 
    then
        echo "No IONIC driver loaded. Loading $DEFAULT_IONIC_DRIVER_PATH"
        insmod $DEFAULT_IONIC_DRIVER_PATH || (dmesg && exit 1)
        sleep 2
    fi

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
        # Collect fw-version information
        if [ -z $fw_version ] ; then
            full_version=`ethtool -i $n`
            fw_version=`ethtool -i $n  | grep firmware-version | awk  '{ print $2 }'`
        fi
    done
    for i in $ifs
    do
        ifconfig $i up
    done
}

dhcp_disable() {
    # Check if it is centos
    os_str=`awk -F= '/^NAME/{print $2}' /etc/os-release`
    os_version=`awk -F= '$1=="VERSION_ID" { print $2 ;}' /etc/os-release | sed -e 's/\([678]\)\../\1/'`
    if [[ $os_str == *"Ubuntu"* ]]; then
        echo "Ubuntu: No need to disable DHCP on Naples IFs"
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
    fi
    # Bringup all interfaces
    for i in $ifs
    do
        ifconfig $i up
    done
}

function setup_legacy_mgmt_ip() {
    echo "Configuring for Legacy FW (Supporting Single Naples)"
    bdf=`lspci -d :1004 | cut -d' ' -f1`
    intmgmt=`ls /sys/bus/pci/devices/0000:$bdf/net/`
    echo "Internal mgmt interface $intmgmt detected at $bdf." 
    dhcp_disable 
    ifconfig $intmgmt $own_ip/24 
    ifconfig $intmgmt
    if [ -n "$no_mgmt" ]; then 
        echo "Skip ping test of internal mgmt interface on host" 
        exit 0 
    fi 
    echo "Attempting to ping target: $trg_ip"
    if ! (ping -c 5 $trg_ip); then 
        ./print-cores.sh 
        exit 1
    fi
}

function setup_pci_mgmt_ip() {
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
    if [ $num_mgmt -gt 0 ] ; then
        own_ip="169.254.XX.2"
        trg_ip="169.254.XX.1"

        for intmgmt in $mgmt_ifs ; do
            pci=`ethtool -i $intmgmt | awk '/bus-info/ { print $2 }'`
            echo "Internal mgmt interface $intmgmt detected at $pci"

            s_hex=`echo $pci | cut -d: -f2 | tr [a-f] [A-F]`
            s_dec=`echo "obase=10; ibase=16; $s_hex" | bc`
            mgmt_ip=`echo $own_ip | sed -e s/XX/$s_dec/`
            mnic_ip=`echo $trg_ip | sed -e s/XX/$s_dec/`

            ifconfig $intmgmt $mgmt_ip/24
            ifconfig $intmgmt
            if [ -n "$no_mgmt" ]; then
                echo "Skip ping test of internal mgmt interface on host"
                exit 0
            fi
            echo "Attempting to ping target: $mnic_ip"
            if ! (ping -c 5 $mnic_ip); then
                ./print-cores.sh
                exit 1
            fi
        done
    fi
}

function setup_mgmt_ip() {
    case $fw_version in 
        1.1.1-E-15)
            echo "Detected Fw $fw_version - running legacy_mgmt_ip"
            setup_legacy_mgmt_ip
            ;;
        *)
            echo "Detected Fw $fw_version - running pci-based mgmt_ip"
            setup_pci_mgmt_ip
            ;;
    esac
}

if [ -n "$skip_install" ]; then
    echo "user requested to skip install"
elif [ -n "$version_only" ]; then
    init_host
    echo "$full_version"
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
    elif [ -n "$mgmt_only" ]; then 
        init_host
        setup_mgmt_ip
    else
        rmmod ionic 2> /dev/null || rc=$?
        cd /naples/
        tar xf drivers-linux-eth.tar.xz
        cd drivers-linux-eth
        ./setup_libs.sh
        ./build.sh
        init_host

        dhcp_disable

        if [ -n "$no_mgmt" ]; then
            echo "Internal mgmt interface is not required."
            exit 0
        fi
        setup_mgmt_ip
    fi
fi

