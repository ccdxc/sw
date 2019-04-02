#/bin/bash
# Script to print ionic driver statistics.
# Usage ./ionic_stats.sh [<num>]
#Interface number, default is first one.
inf=${1:-0}
#Determine the number of queues enabled.
que=`sysctl dev.ionic.$inf.numq | cut -d ":" -f 2`

printf "TxQ       Head     Tail      Comp         Packets            Clean       ReQ      Comp-Err      TSO[IPv4/IPv6]\n"
for((i=0;i<$que;i++))
do
	h=`sysctl dev.ionic.$inf.txq$i.head | cut -d ":" -f 2`
	t=`sysctl dev.ionic.$inf.txq$i.tail | cut -d ":" -f 2`
	c=`sysctl dev.ionic.$inf.txq$i.comp_index | cut -d ":" -f 2`
	#isr=`sysctl dev.ionic.$inf.txq$i.isr | cut -d ":" -f 2`
	pkts=`sysctl dev.ionic.$inf.txq$i.pkts | cut -d ":" -f 2`
	tx_clean=`sysctl dev.ionic.$inf.txq$i.tx_clean | cut -d ":" -f 2`
	tx_reque=`sysctl dev.ionic.$inf.txq$i.tx_requeued | cut -d ":" -f 2`
	comp_err=`sysctl dev.ionic.$inf.txq$i.comp_err | cut -d ":" -f 2`
	tso_ipv4=`sysctl dev.ionic.$inf.txq$i.tso_ipv4 | cut -d ":" -f 2`
	tso_ipv6=`sysctl dev.ionic.$inf.txq$i.tso_ipv6 | cut -d ":" -f 2`
	printf "TxQ%-2d %8d %8d %8d %16d %16d %8d %8d   %16d/%8d\n" $i $h $t $c $pkts $tx_clean $tx_reque $comp_err  $tso_ipv4 $tso_ipv6
done

printf "\nRxQ       Head     Tail      Comp         Packets            ISR      Clean     Comp-Err   LRO(avg length)    Csum-IP           Csum-L4       RSS:Unknown           Ipv4,             TCP4,               UDP4             IPv6,             TCP6,             UDP6\n"

for((i=0;i<$que;i++))
do
	h=`sysctl dev.ionic.$inf.rxq$i.head | cut -d ":" -f 2`
	t=`sysctl dev.ionic.$inf.rxq$i.tail | cut -d ":" -f 2`
	c=`sysctl dev.ionic.$inf.rxq$i.comp_index | cut -d ":" -f 2`
	isr=`sysctl dev.ionic.$inf.rxq$i.isr_count | cut -d ":" -f 2`
	clean=`sysctl dev.ionic.$inf.rxq$i.clean_count | cut -d ":" -f 2`
	pkts=`sysctl dev.ionic.$inf.rxq$i.pkts | cut -d ":" -f 2`
	comp_err=`sysctl dev.ionic.$inf.rxq$i.comp_err | cut -d ":" -f 2`
	lro_queued=`sysctl dev.ionic.$inf.rxq$i.lro_queued | cut -d ":" -f 2`
	lro_flushed=`sysctl dev.ionic.$inf.rxq$i.lro_flushed | cut -d ":" -f 2`
	csum_ip_ok=`sysctl dev.ionic.$inf.rxq$i.csum_ip_ok | cut -d ":" -f 2`
	csum_ip_bad=`sysctl dev.ionic.$inf.rxq$i.csum_ip_bad | cut -d ":" -f 2`
	csum_l4_ok=`sysctl dev.ionic.$inf.rxq$i.csum_l4_ok | cut -d ":" -f 2`
	csum_l4_bad=`sysctl dev.ionic.$inf.rxq$i.csum_l4_bad | cut -d ":" -f 2`
	rss_unknown=`sysctl dev.ionic.$inf.rxq$i.rss_unknown | cut -d ":" -f 2`
	rss_ip4=`sysctl dev.ionic.$inf.rxq$i.rss_ip4 | cut -d ":" -f 2`
	rss_tcp_ip4=`sysctl dev.ionic.$inf.rxq$i.rss_tcp_ip4 | cut -d ":" -f 2`
	rss_udp_ip4=`sysctl dev.ionic.$inf.rxq$i.rss_udp_ip4 | cut -d ":" -f 2`
	rss_ip6=`sysctl dev.ionic.$inf.rxq$i.rss_ip6 | cut -d ":" -f 2`
	rss_tcp_ip6=`sysctl dev.ionic.$inf.rxq$i.rss_tcp_ip6 | cut -d ":" -f 2`
	rss_udp_ip6=`sysctl dev.ionic.$inf.rxq$i.rss_udp_ip6 | cut -d ":" -f 2`
	lro=0
	if [ $lro_flushed -ne 0 ]; then
		lro=$(($lro_queued/$lro_flushed))
	fi
	printf "RxQ%-2d %8d %8d %8d %16d %16d %8d %8d           %2d   %16d/%2d  %16d/%2d RSS: [%d] [%16d, %16d, %16d] [%16d, %16d, %16d]\n" $i $h $t $c $pkts $isr $clean $comp_err $lro $csum_ip_ok $csum_ip_bad $csum_l4_ok $csum_l4_bad $rss_unknown $rss_ip4 $rss_tcp_ip4 $rss_udp_ip4 $rss_ip6 $rss_tcp_ip6 $rss_udp_ip6
done
