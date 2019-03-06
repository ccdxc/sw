#!/bin/bash
#
#  macvlans.sh
#
#  basic macvlan offload tests
#
#  Usage: macvlans.sh <netdev> <dev-ip> <peer-ip>
#
#  Assertions tested:
#    - driver running in scale mode
#    - driver is up and ping works
#    - one queue for PF, N number of lifs for macvlan use
#    - ethtool -k shows offload is available but off by default
#    - macvlan creation not offloaded
#	no log message for offload
#	no extra queues enabled
#    - ethtool -K enables offload
#    - macvlan creation is offloaded
#	log message for offload
#	extra queue enabled
#	traffic flows through offloaded macvlan
#    - N macvlan offloads available
#    - when MV(n-1) is deleted
#	N queues still show in ethtool -S
#	Packet and byte counts for MV(n-1) are 0
#    - after MV(n-1) is deleted, when MV(n) is deleted
#	N-2 now show in ethtool -S
#    - after all MVs are deleted
#	only the 1 Tx/Rx queue for PF shows in ethtool -S
#	traffic still flows through PF
#

get_queue_count() {
	ethtool -S $DEV | egrep -c "rx_.+_bytes:"
}


exit_message() {
	if [ $EXIT_VAL -eq 0 ] ; then
		echo "$0 PASSED"
	else
		echo "$0 FAILED, $EXIT_VAL errors"
	fi

	# put the ip-addr back
	ifconfig $DEV $DEV_IP

	exit $EXIT_VAL
}

echo "$0 $@"

EXIT_VAL=0

DEV=$1
DEV_IP=$2
PEER_IP=$3
if [ -z "$DEV" -o -z "$DEV_IP" -o -z "$PEER_IP" ] ; then
	echo "Usage: $0 <netdev> <dev-ip> <peer-ip>"
	EXIT_VAL=`expr $EXIT_VAL + 1`
	exit_message
fi

# does our device exist
ip link show $DEV >/dev/null
if [ $? -ne 0 ] ; then
	echo "Error: Device $DEV doesn't exist"
	EXIT_VAL=`expr $EXIT_VAL + 1`
	exit_message
fi

# is it running in standard (no scale) mode?
ethtool -k $DEV | grep "l2-fwd-offload: off \[fixed\]"
if [ $? -eq 0 ] ; then
	echo "Skipping test: Device $DEV not in scale mode"
	exit_message
fi

# is the peer ping-able?
ping -c 1 $PEER_IP >/dev/null 2>&1
if [ $? -ne 0 ] ; then
	echo "Error: can't ping peer $PEER_IP"
	EXIT_VAL=`expr $EXIT_VAL + 1`
	exit_message
fi

# does it have the right number of queues by default?
num_queues=`get_queue_count`
if [ $num_queues -ne 1 ] ; then
	echo "Error: TX and RX queue counts from ethtool should be 1, got $num_queues"
	EXIT_VAL=`expr $EXIT_VAL + 1`
fi


# make sure we're starting with offload 'off'
ethtool -k $DEV | grep "l2-fwd-offload: on"
if [ $? -eq 0 ] ; then
	ethtool -K $DEV l2-fwd-offload off
fi

ethtool -k $DEV | grep "l2-fwd-offload: off"
if [ $? -ne 0 ] ; then
	echo "Error: macvlan offload not disabled for start"
	EXIT_VAL=`expr $EXIT_VAL + 1`
fi

# change base device ip-addr so we can use it on the macvlans
ifconfig $DEV 11.11.11.11


# check that we can create a macvlan and it doesn't get offloaded
ip link add mv1 link $DEV type macvlan
ifconfig mv1 $DEV_IP
num_queues=`get_queue_count`
if [ $num_queues -ne 1 ] ; then
	echo "Error: macvlan offloaded when offload disabled? got $num_queues"
	EXIT_VAL=`expr $EXIT_VAL + 1`
fi

ping -c 1 $PEER_IP >/dev/null
if [ $? -ne 0 ] ; then
	echo "Error: ping through normal macvlan failed"
	EXIT_VAL=`expr $EXIT_VAL + 1`
fi

ip link del mv1



# can we enable the offload?
ethtool -K $DEV l2-fwd-offload on
ethtool -k $DEV | grep -q "l2-fwd-offload: on"
if [ $? -ne 0 ] ; then
	echo "Error: macvlan offload can't be enabled"
	EXIT_VAL=`expr $EXIT_VAL + 1`
	exit_message
fi


# check that a new macvlan is offloaded
ip link add mv1 link $DEV type macvlan
ifconfig mv1 $DEV_IP
num_queues=`get_queue_count`
if [ $num_queues -ne 2 ] ; then
	echo "Error: macvlan not offloaded when offload enabled - got $num_queues"
	EXIT_VAL=`expr $EXIT_VAL + 1`
fi

ping -c 1 $PEER_IP >/dev/null
if [ $? -ne 0 ] ; then
	echo "Error: ping through offloaded macvlan failed"
	EXIT_VAL=`expr $EXIT_VAL + 1`
fi

ip link del mv1


# can we create max offloads?

# parse the log line to find numner of lifs
#  [  201.466167] ionic 0000:b5:00.0: nxqs=1 nslaves=126 nqueues=127
pci=`ethtool -i $DEV | grep bus-info | cut "-d " -f2`
line=`dmesg | grep "$pci: nxqs=" | tail -1 | cut -d: -f4`
nxqs=`echo $line | cut "-d " -f1 | cut -d= -f2`
nslaves=`echo $line | cut "-d " -f2 | cut -d= -f2`
nqueues=`echo $line | cut "-d " -f3 | cut -d= -f2`
tot=`expr $nxqs + $nslaves`
if [ $tot -ne $nqueues ] ; then
	echo "Error: macvlan queue counts incorrect: line=$line"
	EXIT_VAL=`expr $EXIT_VAL + 1`
fi


# try to create all the macvlans

echo "Creating $nslaves macvlans..."
cnt=0
while [ $cnt -lt $nslaves ] ; do

	cnt=`expr $cnt + 1`

	mv="mv$cnt"

	ip1=`echo $DEV_IP | cut -d. -f4`
	ip2=`expr $cnt % 256`
	ip3=`expr $cnt / 256`
	ip4="11"
	ip="$ip4.$ip3.$ip2.$ip1"

	ip link add $mv link $DEV type macvlan
	if [ $? -ne 0 ] ; then
		echo "Error: creation of macvlan $mv failed"
		EXIT_VAL=`expr $EXIT_VAL + 1`

		break
	fi

	ifconfig $mv $ip
	if [ $? -ne 0 ] ; then
		echo "Error: ifconfig $mv $ip failed"
		EXIT_VAL=`expr $EXIT_VAL + 1`

		break
	fi

done


num_sqs=`get_queue_count`
num_sqs=`expr $num_sqs - 1`
num_mvs=`ip -o link show type macvlan | wc -l`
if [ $num_sqs -ne $nslaves -o $num_mvs -ne $nslaves ] ; then
	echo "Error: offload count wrong, should be $nslaves: num_mvs=$num_mvs num_sqs=$num_sqs"
	EXIT_VAL=`expr $EXIT_VAL + 1`
fi


# pick one from the middle for a couple of tests
nmv=`expr $num_mvs / 2`
mv="mv$nmv"
echo "Trying $mv..."

# can we ping through the offloaded macvlan?
ifconfig $mv $DEV_IP
ping -c 1 $PEER_IP >/dev/null
if [ $? -ne 0 ] ; then
	echo "Error: ping through offloaded macvlan $mv failed"
	EXIT_VAL=`expr $EXIT_VAL + 1`
fi

# did the queue stats increment off of zero?
rb=`ethtool -S $DEV | grep rx_${nmv}_bytes | cut -d: -f2`
rp=`ethtool -S $DEV | grep rx_${nmv}_pkts | cut -d: -f2`
tb=`ethtool -S $DEV | grep tx_${nmv}_bytes | cut -d: -f2`
tp=`ethtool -S $DEV | grep tx_${nmv}_pkts | cut -d: -f2`
if [ $rb -eq 0 -o $rp -eq 0 -o $tb -eq 0 -o $tp -eq 0 ] ; then
	echo "Error: stats for $mv should be non-zero: rb=$rb rp=$rp tb=$tb tp=$tp"
	EXIT_VAL=`expr $EXIT_VAL + 1`
fi


# delete the mv in the middle of the list
#   queue count should remain the same
#   rx and tx values should go to 0
ip link del $mv
if [ $? -ne 0 ] ; then
	echo "Error: delete of $mv failed"
	EXIT_VAL=`expr $EXIT_VAL + 1`
fi


# the count of macvlans should have gone down by one
num_mvs=`ip -o link show type macvlan | wc -l`
num_expect_mvs=`expr $nslaves - 1`
if [ $num_mvs -ne $num_expect_mvs ] ; then
	echo "Error: delete of $mv: count of mv should be $num_expect_mvs, got $num_mvs"
	EXIT_VAL=`expr $EXIT_VAL + 1`
fi

# the count of macvlan queues should still be the same
# because we removed one from the middle, not the end
num_qs=`get_queue_count`
if [ $num_qs -ne $nqueues ] ; then
	echo "Error: delete of $mv: count of queues should be $nqueues, got $num_qs"
	EXIT_VAL=`expr $EXIT_VAL + 1`
fi

# remove last half of the mvs, queue count should go down
echo "Deleting second half of macvlans"
smv=`expr $nmv + 1`
for i in `seq $smv $nslaves` ; do
	mv="mv$i"
	ip link del $mv
done

# the count of macvlans should have gone down by half
num_mvs=`ip -o link show type macvlan | wc -l`
num_expect_mvs=`expr $nmv - 1`
if [ $num_mvs -ne $num_expect_mvs ] ; then
	echo "Error: delete of half the mvs: count should be $num_expect_mvs, got $num_mvs"
	EXIT_VAL=`expr $EXIT_VAL + 1`
fi

# the count of macvlan queues should be the same as mvs
num_sqs=`get_queue_count`
num_sqs=`expr $num_sqs - 1`
if [ $num_sqs -ne $num_mvs ] ; then
	echo "Error: delete of $mv: count of queues should be $num_mvs, got $num_sqs"
	EXIT_VAL=`expr $EXIT_VAL + 1`
fi


# delete the rest of them
echo "Deleting rest of macvlans"
for i in `seq 1 $num_mvs` ; do
	mv="mv$i"
	ip link del $mv
done

num_mvs=`ip -o link show type macvlan | wc -l`
num_expect_mvs=0
if [ $num_mvs -ne $num_expect_mvs ] ; then
	echo "Error: delete of rest of the mvs: count should be $num_expect_mvs, got $num_mvs"
	EXIT_VAL=`expr $EXIT_VAL + 1`
fi


# restore PF settings
ifconfig $DEV $DEV_IP
ethtool -K $DEV l2-fwd-offload off
ping -c 1 $PEER_IP >/dev/null 2>&1
if [ $? -ne 0 ] ; then
	echo "Error: after restoring PF, can't ping peer $PEER_IP"
	EXIT_VAL=`expr $EXIT_VAL + 1`
	exit_message
fi


exit_message

