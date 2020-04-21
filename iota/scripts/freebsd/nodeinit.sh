#! /usr/local/bin/bash
set -e

own_ip="169.254.0.2"
trg_ip="169.254.0.1"

DEFAULT_IONIC_DRIVER_PATH=/naples/*/sys/modules/ionic/ionic.ko
while [[ "$#" > 0 ]]; do
    case $1 in
        -c|--cleanup) cleanup=1;;
        --mgmt_only) mgmt_only=1;;
        --skip-install) skip_install=1;;
        --version) version_only=1;;
        --own_ip) own_ip=$2; shift;;
        --no-mgmt) no_mgmt=1;;
        --trg_ip) trg_ip=$2; shift;;
        --image) driver_img=$2; shift;;
        *) echo "Unknown parameter passed: $1"; exit 10;;
    esac; shift;
done

rm -f /root/.ssh/known_hosts
chown vm:vm /pensando
ifs=""
fw_version=""

# find all the ionic interfaces and the mgmt interfaces
function init_host() {
    cnt=`kldstat | grep ionic | wc -l`
    if [[ $cnt -eq 0 ]] ; 
    then
        echo "No IONIC driver loaded. Loading $DEFAULT_IONIC_DRIVER_PATH"
        kldload $DEFAULT_IONIC_DRIVER_PATH || (dmesg && exit 11)
        sleep 2
    fi

    pciconf -l | grep chip=0x10041dd8 | while read line
    do
        echo "Found entry: $line"
        flds=($(echo $line | awk -F'@|:' '{ print $1, $3 }'))
        ifn=$(echo ${flds[0]} | sed "s/ion/ionic/g")
        ifs+=" $ifn"
    done

    for i in $ifs
    do
        ifconfig $i up
    done
    fw_version=`sysctl dev.ionic.0.fw_version | awk '{ print $2 }'`
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
    intmgmt=`pciconf -l | grep chip=0x10041dd8 | cut -d'@' -f1 | sed "s/ion/ionic/g"`

    echo "Internal mgmt interface $intmgmt detected at $bdf." 
    # dhcp_disable 
    ifconfig $intmgmt $own_ip/24
    ifconfig $intmgmt 
    if [ -n "$no_mgmt" ]; then 
        echo "Skip ping test of internal mgmt interface on host" 
        exit 0 
    fi 
    echo "Attempting to ping target: $trg_ip"
    if ! (ping -c 5 $trg_ip); then 
        ./print-cores.sh 
        exit 12
    fi
}

function setup_pci_mgmt_ip() {
    # override possible command line setting
    # with default settings if multiple NICs
    own_ip="169.254.XX.2"
    trg_ip="169.254.XX.1"

    pciconf -l | grep chip=0x10041dd8 | while read line
    do
        echo "Found entry: $line"
        flds=($(echo $line | awk -F'@|:' '{ print $1, $3 }'))
        ifn=$(echo ${flds[0]} | sed "s/ion/ionic/g")
        pci_bus=${flds[1]}

        mgmt_ip=`echo $own_ip | sed -e s/XX/$pci_bus/`
        mnic_ip=`echo $trg_ip | sed -e s/XX/$pci_bus/`

        ifconfig $ifn $mgmt_ip/24
        ifconfig $ifn up
        ifconfig $ifn
        if [ -n "$no_mgmt" ]; then
            echo "Skip ping test of internal mgmt interface on host"
            exit 0
        fi
        echo "Attempting to ping target: $mnic_ip"
        if ! (ping -c 5 $mnic_ip); then
            ./print-cores.sh
            exit 13
        fi
    done
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

# Install additional package here.
pkg install -y nmap

if [ -n "$skip_install" ]; then
    echo "user requested to skip install"
elif [ -n "$version_only" ]; then
    init_host
    echo "$full_version"
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
        kldunload ionic 2> /dev/null || rc=$?
        if [ -n "$rc" ] && [ $rc -ne 0 ]; then
            echo "Failed to unload ionic driver. Ignore"
            exit 0
        fi
        echo "Unloaded ionic driver."
    elif [ -n "$mgmt_only" ]; then
        init_host
        setup_mgmt_ip
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

        if [[ ! -z ${driver_img+x} ]] ;
        then
            kldunload ionic 2> /dev/null || rc=$?
            cd /naples
            tar xf ${driver_img}
            tmp="${driver_img##*/}"
            dir_name="${tmp%.tar.xz}"
            cd ${dir_name}
            env OS_DIR=/usr/src ./build.sh
            sleep 2
        fi

        init_host
        # dhcp_disable
        if [ -n "$no_mgmt" ]; then
            echo "Skip ping test of internal mgmt interface on host"
            exit 0
        fi
        setup_mgmt_ip
    fi
fi

