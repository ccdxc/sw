#!/bin/bash
#
#  number-of-queues.sh
#
#  simple tests of the number of queues set up in an ionic device
#
#  Usage: number-of-queues.sh <netdev> <peer-ip>
#
#  Assertions tested:
#    - driver running in default (not scale) mode
#    - driver is up and ping works
#    - by default, one queue for each cpu
#    - ethtool -l shows correct counts
#    - ethtool -L for rx, tx, other fails
#    - ethtool -L for smaller combined succeeds
#	ethtool -l shows change
#	grep of /proc/interrupts shows change
#	ping still works
#	all Rx queues get traffic after change
#    - ethtool -L back to starting count succeeds
#	ethtool -l shows change
#	grep of /proc/interrupts shows change
#	ping still works
#	all Rx queues get traffic after change
#

get_queue_counts() {
	max_queues=`ethtool -l $DEV | grep Combined | cut -d: -f2 | head -1`
	num_queues=`ethtool -l $DEV | grep Combined | cut -d: -f2 | tail -1`
	max_tx=`ethtool -l $DEV | grep TX | cut -d: -f2 | head -1`
	num_tx=`ethtool -l $DEV | grep TX | cut -d: -f2 | tail -1`
	max_rx=`ethtool -l $DEV | grep RX | cut -d: -f2 | head -1`
	num_rx=`ethtool -l $DEV | grep RX | cut -d: -f2 | tail -1`
	num_intrs=`grep -c $DEV /proc/interrupts`
}


exit_message() {
	if [ $EXIT_VAL -eq 0 ] ; then
		echo "$0 PASSED"
	else
		echo "$0 FAILED, $EXIT_VAL errors"
	fi

	exit $EXIT_VAL
}


EXIT_VAL=0

DEV=$1
PEER=$2
if [ -z "$DEV" -o -z "$PEER" ] ; then
	echo "Usage: $0 <netdev> <peer-ip>"
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
ethtool -k $DEV | grep -q "l2-fwd-offload: off \[fixed\]"
if [ $? -ne 0 ] ; then
	echo "Error: Device $DEV not in standard mode"
	EXIT_VAL=`expr $EXIT_VAL + 1`
	exit_message
fi

# is the peer ping-able?
ping -c 1 $PEER >/dev/null 2>&1
if [ $? -ne 0 ] ; then
	echo "Error: can't ping peer $PEER"
	EXIT_VAL=`expr $EXIT_VAL + 1`
	exit_message
fi

# does it have the right number of queues by default?
num_cpus=`lscpu | awk '/^CPU\(s\):/ { print $2 }'`
if [ $num_cpus -gt 16 ] ; then
	num_cpus=16
fi

get_queue_counts

zero_queues=`expr $max_tx + $num_tx + $max_rx + $num_rx`
if [ $zero_queues -ne 0 ] ; then
	echo "Error: TX and RX queue counts from ethtool should be 0, got $zero_queues"
	EXIT_VAL=`expr $EXIT_VAL + 1`
fi

if [ $num_queues -ne $max_queues ] ; then
	echo "Error: Combined queues $num_queues should equal Max Combined queues $max_queues at start of test"
	EXIT_VAL=`expr $EXIT_VAL + 1`
fi

if [ $num_queues -ne $num_cpus ] ; then
	echo "Error: Combined queues $num_queues should equal num cpus $num_cpus"
	EXIT_VAL=`expr $EXIT_VAL + 1`
fi

if [ $num_queues -ne $num_intrs ] ; then
	echo "Error: Combined queues $num_queues should equal num interrupts $num_intrs"
	EXIT_VAL=`expr $EXIT_VAL + 1`
fi



# try cutting queue count down to something small
qc=2
ethtool -L $DEV combined $qc
if [ $? -ne 0 ] ; then
	echo "Error: setting queue count to $qc returned an error"
	EXIT_VAL=`expr $EXIT_VAL + 1`
fi
sleep 1

get_queue_counts

zero_queues=`expr $max_tx + $num_tx + $max_rx + $num_rx`
if [ $zero_queues -ne 0 ] ; then
	echo "Error: TX and RX queue counts from ethtool should be 0, got $zero_queues"
	EXIT_VAL=`expr $EXIT_VAL + 1`
fi

if [ $num_queues -ne $qc ] ; then
	echo "Error: Combined queues $num_queues should be $qc"
	EXIT_VAL=`expr $EXIT_VAL + 1`
fi

if [ $num_queues -ne $num_intrs ] ; then
	echo "Error: Combined queues $num_queues should equal num interrupts $num_intrs"
	EXIT_VAL=`expr $EXIT_VAL + 1`
fi

ping -c 1 $PEER >/dev/null 2>&1
if [ $? -ne 0 ] ; then
	echo "Error: can't ping peer $PEER after changing queue count to $qc"
	EXIT_VAL=`expr $EXIT_VAL + 1`
fi


# try restoring the queue count
qc=$max_queues
ethtool -L $DEV combined $qc
if [ $? -ne 0 ] ; then
	echo "Error: setting queue count to $qc returned an error"
	EXIT_VAL=`expr $EXIT_VAL + 1`
fi
sleep 1

get_queue_counts

zero_queues=`expr $max_tx + $num_tx + $max_rx + $num_rx`
if [ $zero_queues -ne 0 ] ; then
	echo "Error: TX and RX queue counts from ethtool should be 0, got $zero_queues"
	EXIT_VAL=`expr $EXIT_VAL + 1`
fi

if [ $num_queues -ne $qc ] ; then
	echo "Error: Combined queues $num_queues should be $qc"
	EXIT_VAL=`expr $EXIT_VAL + 1`
fi

if [ $num_queues -ne $num_intrs ] ; then
	echo "Error: Combined queues $num_queues should equal num interrupts $num_intrs"
	EXIT_VAL=`expr $EXIT_VAL + 1`
fi

ping -c 1 $PEER >/dev/null 2>&1
if [ $? -ne 0 ] ; then
	echo "Error: can't ping peer $PEER after changing queue count to $qc"
	EXIT_VAL=`expr $EXIT_VAL + 1`
fi

exit_message
