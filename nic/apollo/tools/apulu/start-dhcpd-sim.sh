#!/bin/sh
# Usage: start-dhcpd-sim.sh -p <pipeline>

while getopts ":p" opt; do
    case $opt in
        a) PIPELINE=$OPTARG ;;
    esac
done

CUR_DIR=$( readlink -f $( dirname $0 ) )
source $CUR_DIR/../setup_env_sim.sh $PIPELINE

if [ -f /var/run/dhcpd.pid ];then
    pkill -9 `cat /var/run/dhcpd.pid`
    rm -rf /var/run/dhcpd.pid
fi
rm -rf /var/lib/dhcp/dhcpd.leases
mkdir -p /var/lib/dhcp
touch /var/lib/dhcp/dhcpd.leases

DHCP_PKG_DIR=$PDSPKG_TOPDIR/sdk/third-party/dhcp-server-pkg/x86_64
ip tuntap add dev dhcp_tap mode tap >/dev/null 2>&1
ifconfig dhcp_tap 169.254.0.2 netmask 255.255.255.0

LD_LIBRARY_PATH=$PDSPKG_TOPDIR/build/x86_64/$PIPELINE/lib:$DHCP_PKG_DIR/lib
DHCPD_ARGS="-cf $PDSPKG_TOPDIR/conf/dhcp-server/dhcpd.conf dhcp_tap"
LD_LIBRARY_PATH=$LD_LIBRARY_PATH $DHCP_PKG_DIR/bin/dhcpd $DHCPD_ARGS
if [[ $? != 0 ]]; then
    exit 1
fi
