#! /bin/bash
#set -x
NICDIR="${PWD%/nic/*}/nic"
SW_DIR=`dirname $NICDIR`
SUBNET=Test_Subnet
SUBNET1=10.1.1.0/24
SUBNET2=11.1.1.0/24
SUBNET3=20.1.1.0/24
CONTAINER=CTR
DOL_CFG=/sw/nic/metaswitch/config/dol_ctr
MIB_PY=/sw/nic/third-party/metaswitch/code/comn/tools/mibapi/metaswitch/cam/mib.py
PDSAGENT=/sw/nic/apollo/tools/apulu/start-agent-mock.sh

###################################################
#
#                           Cntr 3
#                             |  10.3 (210.210.3.3)
#                             |
#                  10.1 --------------- 10.2
#   (100.100.1.1) Cntr 1               Cntr 2  (200.200.2.2)
#                  11.1 --------------- 11.2
#
#
###################################################

RR=0
DOL_RUN=0
UNDERLAY=0

CMDARGS=$*
argc=$#
argv=($@)
for (( j=0; j<argc; j++ )); do
    if [ ${argv[j]} == '--rr' ];then
        RR=1
    elif [ ${argv[j]} == '--dolrun' ];then
        DOL_RUN=1
    fi
    if [ ${argv[j]} == '--underlay' ];then
        UNDERLAY=1
    fi
done

# Pull the pensando/nic container image from the registry
# and create a container
# Use custom target instead of docker/shell to skip pull-assets on warmd VM
make docker/build-runtime-ctr

#Cleanup any stale test containers
echo "Cleanup pre-existing test containers"
ids=$(docker ps --format {{.Names}}| grep $CONTAINER)
for id in $ids
do
    echo "Kill container: $id"
    docker kill $id > /dev/null
done

INTF_NAME=`ip link show | grep en.*UP | head -n1 | cut -d ":" -f2 | xargs`
echo "Using interface $INTF_NAME"

#Cleanup mac-vlan interface, if exists
ip link del ${INTF_NAME}.3006 > /dev/null
ip link add link ${INTF_NAME} name ${INTF_NAME}.3006 type vlan id 3006 >/dev/null
ip link set ${INTF_NAME} up > /dev/null
ip link set ${INTF_NAME}.3006 up > /dev/null
ip link del ${INTF_NAME}.3007 > /dev/null
ip link add link ${INTF_NAME} name ${INTF_NAME}.3007 type vlan id 3007 >/dev/null
ip link set ${INTF_NAME} up > /dev/null
ip link set ${INTF_NAME}.3006 up > /dev/null

# delete all existing networks
echo "Cleanup pre-existing test networks"
ids=$(docker network ls --format {{.Name}} | grep $SUBNET)
for id in $ids
do
    echo "Delete network: $id"
    docker network rm $id > /dev/null
done

#create networks with test subnets as per topology
echo "Create network: "$SUBNET"1 $SUBNET1"
docker network create -d bridge "$SUBNET"1 --subnet $SUBNET1 --gateway 10.1.1.254 -o parent=${INTF_NAME}.3007 > /dev/null
echo "Create network: "$SUBNET"2 $SUBNET2"
docker network create -d macvlan --subnet $SUBNET2 --gateway 11.1.1.254 -o parent=${INTF_NAME}.3006 "$SUBNET"2 > /dev/null

for i in {1..3}
do
    if [ "$i" = "3" ] && [ $UNDERLAY == 1 ]; then
        echo "Skip CTR3 in Underlay mode"
        continue
    fi
    docker run -dit --rm -e PYTHONPATH=/sw/nic/third-party/metaswitch/code/comn/tools/mibapi --sysctl net.ipv6.conf.all.disable_ipv6=1 --net="$SUBNET"1 --privileged --name "$CONTAINER"$i -v $SW_DIR:/sw  -w /sw/nic pensando/nic > /dev/null
    id=$(docker ps --format {{.Names}}| grep "$CONTAINER"$i)
    ip=$(docker exec -it "$CONTAINER"$i ip -o -4 addr list eth0 | awk '{print $4}' | cut -d "/" -f 1)
    printf "Container: $id is started with eth0 $ip "
    if [ $i -lt  3 ]; then
        docker network connect "$SUBNET"2 "$CONTAINER"$i > /dev/null
        ip2=$(docker exec -it "$CONTAINER"$i ip -o -4 addr list eth1 | awk '{print $4}' | cut -d "/" -f 1)
        echo "and eth1 $ip2"
    elif [ $RR == 1 ]; then
        # RR has no Linux route programming - so configure explicit route to TEP IPs
        docker exec -dit "$CONTAINER"3 ip route add 100.100.1.1/32 via 10.1.1.1
        docker exec -dit "$CONTAINER"3 ip route add 200.200.2.2/32 via 10.1.1.2
    fi
    docker exec -dit "$CONTAINER"$i cp "$DOL_CFG"$i/fru.json /tmp/fru.json
done
echo ""

rr=0
# Remove interface addresses - controlplane should install this
docker exec -dit "$CONTAINER"1 ip addr del 10.1.1.1/24 dev eth0
docker exec -dit "$CONTAINER"1 ip addr del 11.1.1.1/24 dev eth1
docker exec -dit "$CONTAINER"2 ip addr del 10.1.1.2/24 dev eth0
docker exec -dit "$CONTAINER"2 ip addr del 11.1.1.2/24 dev eth1

if [ $RR == 1 ]; then
    echo "RR testing mode"
    rr=1
fi

if [ $DOL_RUN == 0 ]; then 
    echo "start pdsagent in "$CONTAINER"1"
    ret=0
    docker exec -dit -w "$DOL_CFG"1 "$CONTAINER"1 sh -c "VPP_IPC_MOCK_MODE=1 $PDSAGENT --log-dir ${DOL_CFG}1" || ret=$?
    
    if [ $ret -ne 0 ]; then
        echo "failed to start pdsagent in "$CONTAINER"1: $ret"
    fi
    echo "sleep 30 seconds to let n-base to start in containers"
    t_s=30
    while [ $t_s -gt 0 ]; do
       echo -ne "$t_s\033[0K\r"
       sleep 1
       : $((t_s--))
    done
fi

for i in {2..3}
do
    ret=0
    if [ "$i" = "3" ] && [ $UNDERLAY == 1 ]; then
        echo "Skip CTR3 in Underlay mode"
        continue
    fi
    if [ "$i" = "3" ] && [ $RR == 1 ]; then
        echo "Starting Pegasus in "$CONTAINER"$i"
        docker exec -dit -w "$DOL_CFG"$i -e LD_LIBRARY_PATH=/sw/nic/third-party/metaswitch/output/x86_64/ "$CONTAINER"$i sh -c '/sw/nic/build/x86_64/apulu/capri/bin/pegasus' || ret=$?
    else
        echo "start pdsagent in "$CONTAINER"$i in PDS_MOCK_MODE"
        docker exec -dit -w "$DOL_CFG"$i "$CONTAINER"$i sh -c "PDS_MOCK_MODE=1 $PDSAGENT --log-dir ${DOL_CFG}$i" || ret=$?
    fi
    if [ $ret -ne 0 ]; then
        echo "failed to start pdsagent in "$CONTAINER"$i: $ret"
    fi
    echo "sleep 25 seconds to let n-base to start in containers"
    t_s=25
    while [ $t_s -gt 0 ]; do
       echo -ne "$t_s\033[0K\r"
       sleep 1
       : $((t_s--))
    done
done

if [ $UNDERLAY == 0 ] && [ $RR == 0 ]; then
    docker exec -it "$CONTAINER"3 python $MIB_PY set localhost evpnEntTable evpnEntEntityIndex=2 evpnEntLocalRouterAddressType=inetwkAddrTypeIpv4 evpnEntLocalRouterAddress='0xd2 0xd2 0x3 0x3'
fi

rrcmd='/sw/nic/build/x86_64/apulu/capri/bin/pds_ms_uecmp_rr_grpc_test'
if [ $UNDERLAY = 1 ]; then
    rrcmd='/sw/nic/build/x86_64/apulu/capri/bin/pds_ms_uecmp_rr_grpc_test underlay'
fi

for i in {1..3}
do
    if [ $DOL_RUN == 1 ] && [ "$i" = "1" ]; then
        echo "Skip configuring DUT in DOL_RUN mode"
        continue
    fi
    if [ $UNDERLAY == 1 ] && [ "$i" = "3" ]; then
        echo "Skip configuring CTR3 in Underlay mode"
        continue
    fi
    ret=0
    if [ $RR == 1 ]; then
        echo "push "$DOL_CFG"$i/evpn.json RR config to "$CONTAINER"$i"
        docker exec -it -e CONFIG_PATH="$DOL_CFG"$i  "$CONTAINER"$i sh -c "$rrcmd" || ret=$?
    else   
        echo "push "$DOL_CFG"$i/evpn.json config to "$CONTAINER"$i"
        docker exec -it -e CONFIG_PATH="$DOL_CFG"$i  "$CONTAINER"$i sh -c '/sw/nic/build/x86_64/apulu/capri/bin/pds_ms_uecmp_grpc_test' || ret=$?
    fi
    if [ $ret -ne 0 ]; then
        echo "failed to push config to "$CONTAINER"$i: $ret"
    fi
done

if [ $UNDERLAY == 1 ]; then
    exit
fi

echo "Originate EVPN Type 5 route from "$CONTAINER"2"

docker exec -it "$CONTAINER"2 sh -c "python /sw/nic/third-party/metaswitch/code/comn/tools/mibapi/metaswitch/cam/mib.py set localhost rtmRedistTable rtmRedistFteIndex=2 rtmRedistEntryId=10 rtmRedistRowStatus=createAndGo rtmRedistAdminStat=adminStatusUp  rtmRedistInfoSrc=atgQcProtStatic  rtmRedistInfoDest=atgQcProtBgp rtmRedistDestInstFlt=true rtmRedistDestInst=2  rtmRedistRedistFlag=true"

docker exec -it "$CONTAINER"2 sh -c "python /sw/nic/third-party/metaswitch/code/comn/tools/mibapi/metaswitch/cam/mib.py set localhost rtmStaticRtTable rtmStaticRtFteIndex=2 rtmStaticRtDestAddrType=inetwkAddrTypeIpv4 rtmStaticRtDestAddr=0x80100000 rtmStaticRtDestLen=16 rtmStaticRtNextHopType=inetwkAddrTypeIpv4 rtmStaticRtNextHop=0x0c00000f rtmStaticRtIfIndex=0 rtmStaticRtRowStatus=createAndGo rtmStaticRtAdminStat=adminStatusUp rtmStaticRtOverride=true rtmStaticRtAdminDist=150"

if [ $RR == 0 ]; then
    echo "Originate EVPN Type 5 route from "$CONTAINER"3"

    docker exec -it "$CONTAINER"3 sh -c "python /sw/nic/third-party/metaswitch/code/comn/tools/mibapi/metaswitch/cam/mib.py set localhost rtmRedistTable rtmRedistFteIndex=2 rtmRedistEntryId=10 rtmRedistRowStatus=createAndGo rtmRedistAdminStat=adminStatusUp  rtmRedistInfoSrc=atgQcProtStatic  rtmRedistInfoDest=atgQcProtBgp rtmRedistDestInstFlt=true rtmRedistDestInst=2  rtmRedistRedistFlag=true"

    docker exec -it "$CONTAINER"3 sh -c "python /sw/nic/third-party/metaswitch/code/comn/tools/mibapi/metaswitch/cam/mib.py set localhost rtmStaticRtTable rtmStaticRtFteIndex=2 rtmStaticRtDestAddrType=inetwkAddrTypeIpv4 rtmStaticRtDestAddr=0x80100000 rtmStaticRtDestLen=16 rtmStaticRtNextHopType=inetwkAddrTypeIpv4 rtmStaticRtNextHop=0x0c00000e rtmStaticRtIfIndex=0 rtmStaticRtRowStatus=createAndGo rtmStaticRtAdminStat=adminStatusUp rtmStaticRtOverride=true rtmStaticRtAdminDist=150"
fi

if [ $DOL_RUN == 1 ]; then
    docker exec -it "$CONTAINER"1 sh -c "sudo ./apollo/tools/rundol.sh --pipeline apulu --topo overlay --feature overlay_networking --sub underlay_trig" || ret=$?
    if [ $ret -ne 0 ]; then
        echo "DOL run failed: $ret"
    else
        echo "DOL run passed: $ret"
    fi

    cd $NICDIR && tar -cvzf controlplane_dol_logs.tgz core* *.log metaswitch/config/dol_ctr*/*.log metaswitch/config/dol_ctr*/core*

    grep ++++ $NICDIR/pds-agent.log
    if grep -q "++++ Underlay Pathset.* Num NH 2.*Async reply Success" $NICDIR/pds-agent.log; then
       echo Underlay ECMP found PASS
    else
       echo Underlay ECMP not found FAIL
       ret=1
    fi
    if grep -q "++++ Overlay Pathset.* Num NH 2.*Async reply Success" $NICDIR/pds-agent.log; then
       echo Overlay ECMP found PASS
    else
       echo Overlay ECMP not found FAIL
       ret=1
    fi
    if grep -q "++++ MS Route 128.16.0.0/16.*Async reply Success" $NICDIR/pds-agent.log; then
       echo Type 5 route installed PASS
    else
       echo Type 5 route not installed FAIL
       ret=1
    fi
    if grep -q "++++ BD 1 MAC 00:ee:00:00:00:05.*Async reply Success" $NICDIR/pds-agent.log; then
       echo Type 2 route installed PASS
    else
       echo Type 2 route not installed FAIL
       ret=1
    fi
    exit $ret
else
    sleep 5
    ret=0
    DUTDIR=$NICDIR/metaswitch/config/dol_ctr1
    grep ++++ $DUTDIR/pds-agent.log
    if grep -q "++++ Underlay Pathset.* Num NH 2.*Async reply Success" $DUTDIR/pds-agent.log; then
       echo Underlay ECMP found PASS
    else
       echo Underlay ECMP not found FAIL
       ret=1
    fi
    if [ $RR == 0 ]; then
        if grep -q "++++ Overlay Pathset.* Num NH 2.*Async reply Success" $DUTDIR/pds-agent.log; then
           echo Overlay ECMP found PASS
        else
           echo Overlay ECMP not found FAIL
           ret=1
        fi
    else
        if grep -q "++++ Overlay Pathset.* Num NH 1.*Async reply Success" $DUTDIR/pds-agent.log; then
           echo Overlay Pathset found PASS
        else
           echo Overlay Pathset not found FAIL
           ret=1
        fi
    fi
    if grep -q "++++ MS Route 128.16.0.0/16.*Async reply Success" $DUTDIR/pds-agent.log; then
       echo Type 5 route installed PASS
    else
       echo Type 5 route not installed FAIL
       ret=1
    fi
    if [ $RR == 1 ]; then
        if grep -q "++++ BD 1 MAC 00:12:23:45:67:08.*Async reply Success" $DUTDIR/pds-agent.log; then
           echo Type 2 route installed PASS
        else
           echo Type 2 route not installed FAIL
           ret=1
        fi
    else
        if grep -q "++++ BD 1 MAC 00:ee:00:00:00:05.*Async reply Success" $DUTDIR/pds-agent.log; then
           echo Type 2 route installed PASS
        else
           echo Type 2 route not installed FAIL
           ret=1
        fi
    fi
    exit $ret
fi



# Older way to create Type 5
#Create an connected L3 interface subnet in Tenant VRF to act as nexthop for the prefix so that the Prefix gets advertised to EVPN
#docker exec -it "$CONTAINER"2 python /sw/nic/third-party/metaswitch/code/comn/tools/mibapi/metaswitch/cam/mib.py set localhost limGenIrbInterfaceTable \
#                                     limEntEntityIndex=1 \
#                                     limGenIrbInterfaceBdType=limBridgeDomainEvpn \
#                                     limGenIrbInterfaceBdId=1 \
#                                     limGenIrbInterfaceRowStatus=rowCreateAndGo \
#                                     limGenIrbInterfaceMacAddr="0x00 0x11 0x22 0x33 0x44 0x55" \
#                                     limGenIrbInterfaceAdminStatus=adminStatusUp
#
#docker exec -it "$CONTAINER"2 sh -c "python /sw/nic/third-party/metaswitch/code/comn/tools/mibapi/metaswitch/cam/mib.py set localhost limInterfaceCfgTable \
#                                     limEntEntityIndex=1 \
#                                     limInterfaceCfgIfIndex=1073823745 \
#                                     limInterfaceCfgIpv6Enabled=triStateFalse \
#                                     limInterfaceCfgRowStatus=rowCreateAndGo \
#                                     limInterfaceCfgBindVrfName=0x32"
#
#docker exec -it "$CONTAINER"2 sh -c "python /sw/nic/third-party/metaswitch/code/comn/tools/mibapi/metaswitch/cam/mib.py set localhost limL3InterfaceAddressTable \
#                                     limEntEntityIndex=1 \
#                                     limL3InterfaceAddressIfIndex=1073823745 \
#                                     limL3InterfaceAddressIPAddrType=inetwkAddrTypeIpv4 \
#                                     limL3InterfaceAddressIPAddress=192.168.1.254 \
#                                     limL3InterfaceAddressRowStatus=rowCreateAndGo \
#                                     limL3InterfaceAddressAdminStatus=adminStatusUp \
#                                     limL3InterfaceAddressPrefixLen=24"

#docker exec -it "$CONTAINER"2 sh -c "python /sw/nic/third-party/metaswitch/code/comn/tools/mibapi/metaswitch/cam/mib.py set localhost rtmStaticRtTable rtmStaticRtFteIndex=2 rtmStaticRtDestAddrType=inetwkAddrTypeIpv4 rtmStaticRtDestAddr=0x80100000 rtmStaticRtDestLen=16 rtmStaticRtNextHopType=inetwkAddrTypeIpv4 rtmStaticRtNextHop=0xc0a8010a rtmStaticRtIfIndex=0 rtmStaticRtRowStatus=createAndGo rtmStaticRtAdminStat=adminStatusUp rtmStaticRtOverride=true rtmStaticRtAdminDist=150"

