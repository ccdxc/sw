#/bin/bash
# Script to print and check ionic driver statistics.
#Interface name like "ionic.0", default is first one.
num=${2:-"ionic.0"}

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

# Validate the packet counter between driver and firmware
function check_stats {
	inf=$1
	#Sum all Tx queue packet counts.
	drv_tx_pkts=0
	for i in `sysctl dev.$inf | grep txq | grep pkts | cut -d ":" -f 2`; 
	do 
   		drv_tx_pkts=$((drv_tx_pkts+i));
	done;

	#Sum all Rx queue packet counts.
	drv_rx_pkts=0
	for i in `sysctl dev.$inf | grep rxq | grep pkts | cut -d ":" -f 2`; 
	do 
		drv_rx_pkts=$((drv_rx_pkts+i));
	done;

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
	    echo $inf "Packet count Tx check failed: Port[$fw_tx_pkts] > Driver[$drv_tx_pkts]"
	    status=1
	fi

	#Rx packet count for driver should be less than firmware counter.
	if [ $fw_rx_pkts -lt $drv_rx_pkts ];
    	then
	    echo $inf "Packet count rx check failed: Port[$fw_rx_pkts] < Driver[$drv_rx_pkts]"
	    status=1
	fi

	if [ $status -eq 0 ];
	then
		echo  $inf Packet Tx[drv: $drv_tx_pkts fw: $fw_tx_pkts], Rx[drv: $drv_rx_pkts fw: $fw_rx_pkts] 
	fi

	exit $status
}

case "$1" in
	print)
		print_stats $num
		;;
	check)
		check_stats $num
		;;
	*)
		echo $"Usage: $0 {print|check} ionic.<num>"
		exit 1
esac
