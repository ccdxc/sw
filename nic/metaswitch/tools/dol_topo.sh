#! /bin/bash

CUR_DIR=`pwd`
SW_DIR=`dirname $CUR_DIR`
SUBNET=Test_Subnet
SUBNET1=10.1.1.0/24
SUBNET2=11.1.1.0/24
CONTAINER=CTR

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
	docker run -dit --rm --sysctl net.ipv6.conf.all.disable_ipv6=1 --net="$SUBNET"1 --privileged --name "$CONTAINER"$i -v $SW_DIR:/sw  -v /vol/builds:/vol/builds -w /sw/nic pensando/nic > /dev/null
	id=$(docker ps --format {{.Names}}| grep "$CONTAINER"$i)
	ip=$(docker exec -it "$CONTAINER"$i ip -o -4 addr list eth0 | awk '{print $4}' | cut -d "/" -f 1)
	printf "Container: $id is started with eth0 $ip "
	if [ $i -lt  3 ]; then
		docker network connect "$SUBNET"2 "$CONTAINER"$i > /dev/null
		ip2=$(docker exec -it "$CONTAINER"$i ip -o -4 addr list eth1 | awk '{print $4}' | cut -d "/" -f 1)
		echo "and eth1 $ip2"
	fi
done
echo ""
for i in {2..3}
do
	echo "start pdsagent in "$CONTAINER"$i in PDS_MOCK_MODE"
	ret=0
	docker exec -dit "$CONTAINER"$i sh -c 'PDS_MOCK_MODE=1 /sw/nic/apollo/tools/apulu/start-agent-mock.sh' || ret=$?
	if [ $ret -ne 0 ]; then
		echo "failed to start pdsagent in "$CONTAINER"$i: $ret"
	fi
done
echo "sleep 30 seconds to let n-base to start in containers"
sleep 30
for i in {2..3}
do
	ret=0
	echo "push ctr$i/evpn.json config to "$CONTAINER"$i from "$CONTAINER"1"
	if [ $i -eq 2 ]; then
		docker exec -it "$CONTAINER"1 sh -c 'CONFIG_PATH=/sw/nic/metaswitch/config/dol_ctr2 /sw/nic/build/x86_64/apulu/bin/pds_ms_grpc_test > /dev/null' || ret=$?
		if [ $ret -ne 0 ]; then
			echo "failed to push config to "$CONTAINER"$i: $ret"
		fi
	else
		docker exec -it "$CONTAINER"1 sh -c 'CONFIG_PATH=/sw/nic/metaswitch/config/dol_ctr3 /sw/nic/build/x86_64/apulu/bin/pds_ms_grpc_test > /dev/null' || ret=$?
		if [ $ret != 0 ]; then
			echo "failed to push config to "$CONTAINER"$i: $ret"
		fi
	fi
done
