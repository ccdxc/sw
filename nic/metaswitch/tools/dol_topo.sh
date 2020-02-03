#! /bin/bash
#set -x
CUR_DIR=`pwd`
SW_DIR=`dirname $CUR_DIR`
SUBNET=Test_Subnet
SUBNET1=10.1.1.0/24
SUBNET2=11.1.1.0/24
SUBNET3=20.1.1.0/24
CONTAINER=CTR
DOL_CFG=/sw/nic/metaswitch/config/dol_ctr
MIB_PY=/sw/nic/third-party/metaswitch/code/comn/tools/mibapi/metaswitch/cam/mib.py

###################################################
#
#                      Cntr 3
#                         |  10.3
#                         |
#              10.1 --------------- 10.2
#   (1.1.1.1) Cntr 1               Cntr 2  (2.2.2.2)
#              11.1 --------------- 11.2
#
#
###################################################

#Cleanup any stale test containers
echo "Cleanup pre-existing test containers"
ids=$(docker ps --format {{.Names}}| grep $CONTAINER)
for id in $ids
do
	echo "Kill container: $id"
	docker kill $id > /dev/null
done
#Cleanup mac-vlan interface, if exists
vconfig rem eno1.3006 > /dev/null
vconfig add eno1 3006 >/dev/null
ifconfig eno1.3006 up > /dev/null
vconfig rem eno1.3007 > /dev/null
vconfig add eno1 3007 >/dev/null
ifconfig eno1.3007 up > /dev/null
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
docker network create -d bridge "$SUBNET"1 --subnet $SUBNET1 --gateway 10.1.1.254 -o parent=eno1.3007 > /dev/null
echo "Create network: "$SUBNET"2 $SUBNET2"
docker network create -d macvlan --subnet $SUBNET2 --gateway 11.1.1.254 -o parent=eno1.3006 "$SUBNET"2 > /dev/null

for i in {1..3}
do
	docker run -dit --rm -e PYTHONPATH=/sw/nic/third-party/metaswitch/code/comn/tools/mibapi --sysctl net.ipv6.conf.all.disable_ipv6=1 --net="$SUBNET"1 --privileged --name "$CONTAINER"$i -v $SW_DIR:/sw  -v /vol/builds:/vol/builds -w /sw/nic pensando/nic > /dev/null
	id=$(docker ps --format {{.Names}}| grep "$CONTAINER"$i)
	ip=$(docker exec -it "$CONTAINER"$i ip -o -4 addr list eth0 | awk '{print $4}' | cut -d "/" -f 1)
	printf "Container: $id is started with eth0 $ip "
	if [ $i -lt  3 ]; then
		docker network connect "$SUBNET"2 "$CONTAINER"$i > /dev/null
		ip2=$(docker exec -it "$CONTAINER"$i ip -o -4 addr list eth1 | awk '{print $4}' | cut -d "/" -f 1)
		echo "and eth1 $ip2"
    elif [ "$1" == "rr" ]; then
        # RR has no Linux route programming - so configure explicit route to TEP IPs
        docker exec -dit "$CONTAINER"3 ip route add 1.1.1.1/32 via 10.1.1.1
        docker exec -dit "$CONTAINER"3 ip route add 2.2.2.2/32 via 10.1.1.2
	fi
    docker exec -dit "$CONTAINER"$i cp "$DOL_CFG"$i/fru.json /tmp/fru.json
done
echo ""

rr=0

if [ "$1" == "rr" ]; then
    echo "RR testing mode"
    rr=1
fi

# TODO: Change to start Container 1 (DUT) alone in PDS non-mock mode
echo "start pdsagent in "$CONTAINER"1 in PDS_MOCK_MODE"
ret=0
docker exec -dit -w "$DOL_CFG"1 "$CONTAINER"1 sh -c 'VPP_IPC_MOCK_MODE=1 /sw/nic/apollo/tools/apulu/start-agent-mock.sh' || ret=$?
#docker exec -dit -w "$DOL_CFG"1 "$CONTAINER"1 sh -c 'PDS_MOCK_MODE=1 VPP_IPC_MOCK_MODE=1 /sw/nic/apollo/tools/apulu/start-agent-mock.sh' || ret=$?

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

for i in {2..3}
do
	ret=0
    if [ "$i" = "3" ] && [ "$rr" = "1" ]; then
        echo "Starting Pegasus in "$CONTAINER"$i"
#  TODO: Change to Pegasus
#    docker exec -dit -w "$DOL_CFG"$i -e LD_LIBRARY_PATH=/sw/nic/third-party/metaswitch/output/x86_64/debug/ "$CONTAINER"$i sh -c '/sw/nic/build/x86_64/apulu/bin/pegasus' || ret=$?
   	docker exec -dit -w "$DOL_CFG"$i "$CONTAINER"$i sh -c 'PDS_MOCK_MODE=1 /sw/nic/apollo/tools/apulu/start-agent-mock.sh' || ret=$?
    else
	    echo "start pdsagent in "$CONTAINER"$i in PDS_MOCK_MODE"
    	docker exec -dit -w "$DOL_CFG"$i "$CONTAINER"$i sh -c 'PDS_MOCK_MODE=1 /sw/nic/apollo/tools/apulu/start-agent-mock.sh' || ret=$?
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

docker exec -it "$CONTAINER"3 python $MIB_PY set localhost evpnEntTable evpnEntEntityIndex=2 evpnEntLocalRouterAddressType=inetwkAddrTypeIpv4 evpnEntLocalRouterAddress='0x3 0x3 0x3 0x3'

for i in {1..3}
do
	ret=0
    if [ "$rr" = "1" ]; then
        echo "push "$DOL_CFG"$i/evpn.json RR config to "$CONTAINER"$i"
    	docker exec -it -e CONFIG_PATH="$DOL_CFG"$i  "$CONTAINER"$i sh -c '/sw/nic/build/x86_64/apulu/bin/pds_ms_uecmp_rr_grpc_test' || ret=$?
    else    
        echo "push "$DOL_CFG"$i/evpn.json config to "$CONTAINER"$i"
    	docker exec -it -e CONFIG_PATH="$DOL_CFG"$i  "$CONTAINER"$i sh -c '/sw/nic/build/x86_64/apulu/bin/pds_ms_uecmp_grpc_test' || ret=$?
    fi
	if [ $ret -ne 0 ]; then
		echo "failed to push config to "$CONTAINER"$i: $ret"
	fi
done

	echo "Originate EVPN Type 5 route from "$CONTAINER"2"
   docker exec -it "$CONTAINER"2 sh -c "python /sw/nic/third-party/metaswitch/code/comn/tools/mibapi/metaswitch/cam/mib.py set localhost rtmRedistTable rtmRedistFteIndex=2 rtmRedistEntryId=10 rtmRedistRowStatus=createAndGo rtmRedistAdminStat=adminStatusUp  rtmRedistInfoSrc=atgQcProtStatic  rtmRedistInfoDest=atgQcProtBgp rtmRedistDestInstFlt=true rtmRedistDestInst=2  rtmRedistRedistFlag=true"
   docker exec -it "$CONTAINER"2 sh -c "python /sw/nic/third-party/metaswitch/code/comn/tools/mibapi/metaswitch/cam/mib.py set localhost rtmStaticRtTable rtmStaticRtFteIndex=2 rtmStaticRtDestAddrType=inetwkAddrTypeIpv4 rtmStaticRtDestAddr=0x80100000 rtmStaticRtDestLen=16 rtmStaticRtNextHopType=inetwkAddrTypeIpv4 rtmStaticRtNextHop=0xc0a8010a rtmStaticRtIfIndex=0 rtmStaticRtRowStatus=createAndGo rtmStaticRtAdminStat=adminStatusUp rtmStaticRtOverride=true rtmStaticRtAdminDist=150"
    if [ "$rr" != "1" ]; then
	echo "Originate EVPN Type 5 route from "$CONTAINER"3"
    docker exec -it "$CONTAINER"3 sh -c "python /sw/nic/third-party/metaswitch/code/comn/tools/mibapi/metaswitch/cam/mib.py set localhost rtmRedistTable rtmRedistFteIndex=2 rtmRedistEntryId=10 rtmRedistRowStatus=createAndGo rtmRedistAdminStat=adminStatusUp  rtmRedistInfoSrc=atgQcProtStatic  rtmRedistInfoDest=atgQcProtBgp rtmRedistDestInstFlt=true rtmRedistDestInst=2  rtmRedistRedistFlag=true"
    docker exec -it "$CONTAINER"3 sh -c "python /sw/nic/third-party/metaswitch/code/comn/tools/mibapi/metaswitch/cam/mib.py set localhost rtmStaticRtTable rtmStaticRtFteIndex=2 rtmStaticRtDestAddrType=inetwkAddrTypeIpv4 rtmStaticRtDestAddr=0x80100000 rtmStaticRtDestLen=16 rtmStaticRtNextHopType=inetwkAddrTypeIpv4 rtmStaticRtNextHop=0xc0a8010a rtmStaticRtIfIndex=0 rtmStaticRtRowStatus=createAndGo rtmStaticRtAdminStat=adminStatusUp rtmStaticRtOverride=true rtmStaticRtAdminDist=150"
    fi
