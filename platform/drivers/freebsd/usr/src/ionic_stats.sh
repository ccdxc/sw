#/bin/bash
# Script to print and check ionic driver statistics.
#Interface name like "ionic.0", default is first one.
ionic_inf="ionic.0"
app_tx_bytes=0
app_rx_bytes=0
drv_tx_bytes=0
drv_rx_bytes=0

# Print all TxQ stats from driver.
function print_tx_stats {
	inf=$1
	#Determine the number of queues enabled.
	que=`sysctl dev.$inf.numq | cut -d ":" -f 2`

	printf "TxQ       Head     Tail      Comp         Packets            "
	printf "Clean       ReQ      Comp-Err      TSO[IPv4/IPv6]\n"
	for((i=0;i<$que;i++))
	do
		head=`sysctl dev.$inf.txq$i.head | cut -d ":" -f 2`
		tail=`sysctl dev.$inf.txq$i.tail | cut -d ":" -f 2`
		comp=`sysctl dev.$inf.txq$i.comp_index | cut -d ":" -f 2`
		#isr=`sysctl dev.$inf.txq$i.isr | cut -d ":" -f 2`
		pkts=`sysctl dev.$inf.txq$i.pkts | cut -d ":" -f 2`
		tx_clean=`sysctl dev.$inf.txq$i.tx_clean | cut -d ":" -f 2`
		tx_reque=`sysctl dev.$inf.txq$i.tx_requeued | cut -d ":" -f 2`
		comp_err=`sysctl dev.$inf.txq$i.comp_err | cut -d ":" -f 2`
		tso_ipv4=`sysctl dev.$inf.txq$i.tso_ipv4 | cut -d ":" -f 2`
		tso_ipv6=`sysctl dev.$inf.txq$i.tso_ipv6 | cut -d ":" -f 2`
		format="TxQ%-2d %8d %8d %8d %16d %16d %8d %8d   %16d/%8d\n"
		printf "$format" $i $head $tail $comp $pkts $tx_clean $tx_reque $comp_err \
			$tso_ipv4 $tso_ipv6
	done
}

#Print all RxQ stats from driver.
function print_rx_stats {
	inf=$1
	#Determine the number of queues enabled.
	que=`sysctl dev.$inf.numq | cut -d ":" -f 2`
	printf "\nRxQ       Head     Tail      Comp         Packets            ISR      "
	printf "Clean     Comp-Err   LRO(avg length)       Csum-IP               Csum-L4"
	printf "    RSS:Unknown     Ipv4,             TCP4,               UDP4          "
        printf "IPv6,             TCP6,		UDP6\n"

	for((i=0;i<$que;i++))
	do
		head=`sysctl dev.$inf.rxq$i.head | cut -d ":" -f 2`
		tail=`sysctl dev.$inf.rxq$i.tail | cut -d ":" -f 2`
		comp=`sysctl dev.$inf.rxq$i.comp_index | cut -d ":" -f 2`
		isr=`sysctl dev.$inf.rxq$i.isr_count | cut -d ":" -f 2`
		clean=`sysctl dev.$inf.rxq$i.clean_count | cut -d ":" -f 2`
		pkts=`sysctl dev.$inf.rxq$i.pkts | cut -d ":" -f 2`
		comp_err=`sysctl dev.$inf.rxq$i.comp_err | cut -d ":" -f 2`
		lro_queued=`sysctl dev.$inf.rxq$i.lro_queued | cut -d ":" -f 2`
		lro_flushed=`sysctl dev.$inf.rxq$i.lro_flushed | cut -d ":" -f 2`
		csum_ip_ok=`sysctl dev.$inf.rxq$i.csum_ip_ok | cut -d ":" -f 2`
		csum_ip_bad=`sysctl dev.$inf.rxq$i.csum_ip_bad | cut -d ":" -f 2`
		csum_l4_ok=`sysctl dev.$inf.rxq$i.csum_l4_ok | cut -d ":" -f 2`
		csum_l4_bad=`sysctl dev.$inf.rxq$i.csum_l4_bad | cut -d ":" -f 2`
		rss_unknown=`sysctl dev.$inf.rxq$i.rss_unknown | cut -d ":" -f 2`
		rss_ip4=`sysctl dev.$inf.rxq$i.rss_ip4 | cut -d ":" -f 2`
		rss_tcp_ip4=`sysctl dev.$inf.rxq$i.rss_tcp_ip4 | cut -d ":" -f 2`
		rss_udp_ip4=`sysctl dev.$inf.rxq$i.rss_udp_ip4 | cut -d ":" -f 2`
		rss_ip6=`sysctl dev.$inf.rxq$i.rss_ip6 | cut -d ":" -f 2`
		rss_tcp_ip6=`sysctl dev.$inf.rxq$i.rss_tcp_ip6 | cut -d ":" -f 2`
		rss_udp_ip6=`sysctl dev.$inf.rxq$i.rss_udp_ip6 | cut -d ":" -f 2`

		lro=0
		if [ $lro_flushed -ne 0 ]; then
			lro=$(($lro_queued/$lro_flushed))
		fi

		format="RxQ%-2d %8d %8d %8d %16d %16d %8d %8d           %2d   %16d/%2d  %16d/%2d "
		printf "$format" $i $head $tail $comp $pkts $isr $clean $comp_err $lro $csum_ip_ok \
			$csum_ip_bad $csum_l4_ok   $csum_l4_bad 

		format_rss="RSS: [%d] [%16d, %16d, %16d] [%16d, %16d, %16d]\n"
		printf "$format_rss" $rss_unknown $rss_ip4 $rss_tcp_ip4 $rss_udp_ip4 $rss_ip6 \
			$rss_tcp_ip6  $rss_udp_ip6
	done
}

# Print Tx and Rx queue stats.
function print_stats {
	print_tx_stats $1
	print_rx_stats $1
}

function get_drv_stats {
	inf=$1
	#Sum all Tx queue packet counts.
	drv_tx_pkts=0
	for i in `sysctl dev.$inf | grep txq | grep pkts | cut -d ":" -f 2`;
	do
		drv_tx_pkts=$((drv_tx_pkts+i));
	done;

	#Sum all Tx queue byte counts.
	for i in `sysctl dev.$inf | grep txq | grep bytes | cut -d ":" -f 2`;
	do
		drv_tx_bytes=$((drv_tx_pkts+i));
	done;

	#Sum all Rx queue packet counts.
	drv_rx_pkts=0
	for i in `sysctl dev.$inf | grep rxq | grep pkts | cut -d ":" -f 2`;
	do
		drv_rx_pkts=$((drv_rx_pkts+i));
	done;

	#Sum all Rx queue byte counts.
	for i in `sysctl dev.$inf | grep rxq | grep pkts | cut -d ":" -f 2`;
	do
		drv_rx_bytes=$((drv_rx_pkts+i));
	done;
}

# Validate the packet counter between driver and firmware
function check_fw_stats {
	inf=$1

	#Sum all Tx unicast, multi-cast and broadcast packets.
	fw_tx_pkts=0
	for i in `sysctl dev.$inf.fw | grep cast_packets | grep tx | cut -d ":" -f 2`;
	do
		fw_tx_pkts=$((fw_tx_pkts+i));
	done;

	#Sum all Rx unicast, multi-cast and broadcast packets.
	fw_rx_pkts=0
	for i in `sysctl dev.$inf.fw | grep cast_packets | grep rx | cut -d ":" -f 2`;
	do
		fw_rx_pkts=$((fw_rx_pkts+i));
	done;


	status=0
	#Tx packet count for driver should be greater than firmware counter.
	if [ $fw_tx_pkts -gt $drv_tx_pkts ];
	then
		echo $inf "FAILED: Packet count Tx check Port[$fw_tx_pkts] > Driver[$drv_tx_pkts]"
		status=1
	fi

	#Rx packet count for driver should be less than firmware counter.
	if [ $fw_rx_pkts -lt $drv_rx_pkts ];
	then
		echo $inf "FAILED: Packet count rx check Port[$fw_rx_pkts] < Driver[$drv_rx_pkts]"
		status=1
	fi

	if [ $status -eq 0 ];
	then
		echo  $inf Packet Tx[drv: $drv_tx_pkts fw: $fw_tx_pkts], Rx[drv: $drv_rx_pkts fw: $fw_rx_pkts]
	fi

	#Fw update stats area every 200ms so fw and driver count may not match
	# if there is active traffic.
	#exit $status
}

# Check if any bad checksum got reported.
function check_rx_csum {
	inf=$1
	drv_rx_csum_bad=0
	for i in `sysctl dev.$inf | grep csum | grep bad | cut -d ":" -f 2`;
	do
		drv_rx_csum_bad=$((idrv_rx_csum_bad+i));
	done;

	if [ $drv_rx_csum_bad -ne 0 ];
	then
		echo 'Detected bad rx csum ($drv_rx_csum_bad) on $inf'
		sysctl dev.$inf | grep csum | grep bad | grep -v ": 0"
		exit 1
	fi
}

# Check if any error reported by MAC or port.
function check_mac_stats {
	inf=$1
	mac_err_cnt=0
	for i in `sysctl dev.$inf.mac | grep -e bad -e stomped -e jabber -e oversize -e undersized | cut -d ":" -f 2`
	do
		mac_err_cnt=$((mac_err_cnt+i));
	done;

	if [ $mac_err_cnt -ne 0 ];
	then
		echo 'MAC error count($mac_err_cnt) on $inf'
		sysctl dev.$inf.mac | grep -e bad -e stomped -e jabber -e oversize -e undersized | grep -v ": 0"
		exit 1
	fi
}

function check_intf {
	inf=$1

	if [[ $inf != "ionic"* ]];
	then
		echo $inf is not Pensando NIC.
		exit 1
	fi

	if ! sysctl dev.$inf > /dev/null 2>&1
	then
		echo $inf not found, Pendsando PCI devices:
		pciconf -l | grep 1dd8
		exit 1
	fi
}

# Make sure iperf bytes count is less than what driver reported.
function check_app_bytes {
	status=0
	if [ $app_tx_bytes -gt $drv_tx_bytes ];
	then
		echo "$ionic_inf FAILED: Tx App $app_tx_bytes byes > driver $drv_tx_bytes"
		status=1
	fi
	if [ $app_rx_bytes -gt $drv_rx_bytes ];
	then
		echo "$ionic_inf FAILED: Rx App $app_rx_bytes byes > driver $drv_rx_bytes"
		status=1
	fi

	exit $status
}

function usage {
	echo "$0 options"
	echo "  -c Check various statistics"
	echo "  -i<interface>, e.g. interface is ionic.0, ionic.1 etc"
	echo "  -r<byte count>, Number of bytes received as reported by iperf"
	echo "  -t<byte count>, Number of bytes transmitted as reported by iperf"
	echo " e.g. $0 -iionic.0 to print stats for ionic0"
	echo "      $0 -c -iionic.1 to check various stats for ionic1"
	exit 1
}

is_check_cmd=0
while getopts chi:r:t: option
do
	case "${option}" in
	c) is_check_cmd=1 ;;
	i) ionic_inf=${OPTARG} ;;
	r) app_rx_bytes=${OPTARG} ;;
	t) app_tx_bytes=${OPTARG} ;;
	*) usage ;;
	esac
done


check_intf $ionic_inf
if [ $is_check_cmd -ne 0 ];
then
	# Read driver stats.
	get_drv_stats $ionic_inf
	# Validate driver stats against fw
	check_fw_stats $ionic_inf
	# make sure we didn't see bad csum
	check_rx_csum $ionic_inf
	check_mac_stats $ionic_inf
	check_app_bytes
else
	print_stats $ionic_inf
fi
