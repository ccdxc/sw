# VPP
VPP is used as software dataplane for handling exception packets which can't be handled in P4.

## Basic Commands

###  Interface commands:
To login to vpp shell:
```
vppctl
```
Alternatively to run any VPP command without login to shell:
```
vppctl <vpp command>
```

###  Interface commands:
To check interface admin state/counters:
```
show interface
```
To check interface link state/hardware counters:
```
show hardware-interface
```
Clear interface statistics:
```
clear interfaces [interface]
```
Clear interface hardware statistics:
```
clear hardware-interfaces [interface]
```
Sample output for the commands:
```
vpp# show interface                                                            
              Name               Idx    State  MTU (L3/IP4/IP6/MPLS)     Counter          Count     
cpu_mnic0                         1      up          9000/0/0/0     rx packets              11952690
                                                                    rx bytes              1040151096
                                                                    tx packets               8388606
                                                                    tx bytes               296678070
                                                                    drops                    3564084
local0                            0     down          0/0/0/0       

vpp# show hardware-interfaces 
              Name                Idx   Link  Hardware
cpu_mnic0                          1     up   cpu_mnic0
  Link speed: unknown
  Ethernet address 00:ae:cd:00:13:33
  Pensando Naples
    carrier up full duplex mtu 9194 
    flags: admin-up pmd maybe-multiseg rx-ip4-cksum
    rx: queues 3 (max 4), desc 32768 (min 16 max 32768 align 1)
    tx: queues 4 (max 4), desc 4096 (min 16 max 32768 align 1)
    module: unknown
    max rx packet len: 9208
    promiscuous: unicast off all-multicast on
    vlan offload: strip off filter off qinq off
    rx offload avail:  vlan-strip ipv4-cksum udp-cksum tcp-cksum vlan-filter 
                       jumbo-frame 
    rx offload active: ipv4-cksum jumbo-frame 
    tx offload avail:  vlan-insert ipv4-cksum udp-cksum tcp-cksum sctp-cksum 
                       tcp-tso multi-segs 
    tx offload active: multi-segs 
    rss avail:         ipv4 ipv4-tcp ipv4-udp ipv6 ipv6-tcp ipv6-udp 
    rss active:        ipv4 ipv4-tcp ipv4-udp ipv6 ipv6-tcp ipv6-udp 
    tx burst function: ionic_xmit_pkts
    rx burst function: ionic_recv_pkts
                                                                               
    rx frames ok                                    18871165
    rx bytes ok                                   1653345747
    rx errors                                       10133536
    extended stats:
      rx good packets                               18871165
      rx good bytes                               1653345747
      rx errors                                     10133536
local0                             0    down  local0
  Link speed: unknown
  local
```

### Packet trace
To enable packet trace:
```
trace add <input-node-name> 50
```
Since we use DPDK interface for receiving packets from P4, input-node-name should be dpdk-input.

Sample command output to trace 50 packets which are received on dpdk-interfaces, display trace and clear trace:
```
vpp# trace add dpdk-input  50
vpp# show trace                                                                
------------------- Start of thread 0 vpp_main -------------------
No packets in trace buffer
------------------- Start of thread 1 vpp_wk_0 -------------------
Packet 1

16:47:50:346672: dpdk-input
  cpu_mnic0 rx queue 0
  buffer 0x121db8: current data 0, length 81, free-list 0, clone-count 0, totlen-nifb 0, trace 0x0
                   ext-hdr-valid 
                   l4-cksum-computed l4-cksum-correct 
  PKT MBUF: port 0, nb_segs 1, pkt_len 81
    buf_len 2176, data_len 81, ol_flags 0x2, data_off 128, phys_addr 0x51276e80
    packet_type 0x211 l2_len 0 l3_len 0 outer_l2_len 0 outer_l3_len 0
    rss 0xa978a978 fdir.hi 0x0 fdir.lo 0xa978a978
    Packet Offload Flags
      PKT_RX_RSS_HASH (0x0002) RX packet with RSS hash result
    Packet Types
      RTE_PTYPE_L2_ETHER (0x0001) Ethernet packet
      RTE_PTYPE_L3_IPV4 (0x0010) IPv4 packet without extension headers
      RTE_PTYPE_L4_UDP (0x0200) UDP packet
  0x4100: 8f:23:f0:76:1b:2d -> 00:40:00:85:00:00
16:47:50:346710: pds-p4cpu-hdr-lookup
  Flags[0x85], flow_hash[0x8f23f076], l2_offset[27], l3_offset[45] l4_offset[65], vnic[0]
16:47:50:346719: pds-ip4-flow-program
  iFlow: src[2.0.0.4], dst[2.0.0.9], IP_Proto[UDP], src_port[4675], dst_port[6336]
rFlow: src[2.0.0.9], dst[2.0.0.4], IP_Proto[UDP], src_port[6336], dst_port[4675]
16:47:50:346742: pds-fwd-flow
  Out Interface cpu_mnic0
vpp# clear trace 
vpp# show trace 
------------------- Start of thread 0 vpp_main -------------------
No packets in trace buffer
------------------- Start of thread 1 vpp_wk_0 -------------------
No packets in trace buffer
------------------- Start of thread 2 vpp_wk_1 -------------------
No packets in trace buffer
------------------- Start of thread 3 vpp_wk_2 -------------------
No packets in trace buffer
```
### VPP Node counters
In VPP, each node can have its own counters which will be useful for debugging/telemetry. Below command displays all node counters.
```
show node counter
```
To clear node counters, use below command.
```
clear node counters
```
Sample output for flow programming:
```
vpp# show node counters 
   Count                    Node                  Reason
   3417778          pds-p4cpu-hdr-lookup          IPv4 flow packets
   1719141          pds-p4cpu-hdr-lookup          IPv6 flow packets
   3858450          pds-ip4-flow-program          Flow programming success
   1488554          pds-ip4-flow-program          Flow programming failed
   3352856          pds-ip6-flow-program          Flow programming success
     42713          pds-ip6-flow-program          Flow programming failed
   1531267               dpdk-input               no error
   2299829          pds-p4cpu-hdr-lookup          IPv4 flow packets
   1155742          pds-p4cpu-hdr-lookup          IPv6 flow packets
   2597390          pds-ip4-flow-program          Flow programming success
   1001134          pds-ip4-flow-program          Flow programming failed
   2253376          pds-ip6-flow-program          Flow programming success
     29054          pds-ip6-flow-program          Flow programming failed
   1030188               dpdk-input               no error
   2236016          pds-p4cpu-hdr-lookup          IPv4 flow packets
   1124184          pds-p4cpu-hdr-lookup          IPv6 flow packets
   2522962          pds-ip4-flow-program          Flow programming success
    974535          pds-ip4-flow-program          Flow programming failed
   2192180          pds-ip6-flow-program          Flow programming success
     28094          pds-ip6-flow-program          Flow programming failed
   1002629               dpdk-input               no error
```
  ## VPP Flow Plugin

This plugin handles flow miss packets sent to ARM CPU from P4 and programs flow entry/action into hardware. Following are the VPP graph nodes which are added by this plugin:

|Node name                          |Description                         |
|-------------------------------|-----------------------------|
|pds-p4cpu-hdr-lookup            |Parses P4-CPU headers and decides the next node to process the packet.            |
|pds-ip4-flow-program            |Parses native IPv4 flow packets and installs a flow entry in IPv4 flow table using FTLv4 library.             |
|pds-ip6-flow-program|Parses native IPv6 flow packets and installs a flow entry in IPv4 flow table using FTLv6 library.|
|pds-tunnel-ip4-flow-program |Parses inner IPv4 headers of a tunnelled flow packet and installs a flow entry in IPv4 flow table using FTLv4 library.|
|pds-tunnel-ip6-flow-program|Parses inner IPv6 headers of a tunnelled flow packet and installs a flow entry in IPv6 flow table using FTLv6 library.|
|pds-session-program|Programs flow actions in session table. This is valid only on Artemis Pipeline.|
|pds-fwd-flow|Adds required CPU-P4 transmit headers and forwards packet on cpu_mni0 DPDK interface to reinject the packet to P4.|

Below diagram shows packet path for a flow miss packet among VPP graph nodes:

![Packet path](./flow/flow_plugin.png)

## Commands

Following sections describes VPP shell commands added by flow plugin.

### Dump flow table info to file
Using below command, complete flow table can be dumped to a file. Note that depending on number of active entries in table, command can take long time to complete and also may require few GBs worth disk space.
```
dump flow entries file <absolute file path to dump hw entries> [detail] [ip4 | ip6]
```
Using option - detail, along with every field of flow table entry even session table data will be dumped.
Sample output of file:
```
          SrcIP                   DstIP        SrcPort DstPort Proto   Vnic     Session
           -----                   -----        -----   ------- ------- -----       ----
         2.0.0.5                 2.0.0.9         5210    8099    17        0     4709693
         2.0.0.9                 2.0.0.6         1425    2386    17        0     4062400
         2.0.0.4                 2.0.0.9         4490    2382    17        0     4917380
     2019::200:1             2019::200:9         5540    3611    17        0     86729
     2019::200:9             2019::200:1         8759    2097    17        0     86728
     2019::200:1             2019::200:9         2097    8759    17        0     86728

```
Sample output of file with detail option:
```
more_hints:0 more_hashes:0 hint2:0 hash2:0 hint1:2635429 hash1:0x1b9 sport:5210 dport:8099 src:02000005 dst:02000009 local_vnic_tag:0 proto:17 flow_role:0 session_index:4709693 nexthop_group_index:0 entry_valid:1 Session data: 0000000000000000000000000000000000000000000000000000000000000000
more_hints:0 more_hashes:0 hint2:0 hash2:0 hint1:0 hash1:0 sport:1425 dport:2386 src:02000009 dst:02000006 local_vnic_tag:0 proto:17 flow_role:0 session_index:4062400 nexthop_group_index:0 entry_valid:1 Session data: 0000000000000000000000000000000000000000000000000000000000000000
more_hints:1527094 more_hashes:1 hint2:1372844 hash2:0x66 hint1:245311 hash1:0x28 sport:4490 dport:2382 src:02000004 dst:02000009 local_vnic_tag:0 proto:17 flow_role:0 session_index:4917380 nexthop_group_index:0 entry_valid:1 Session data: 0000000000000000000000000000000000000000000000000000000000000000
[#/msb/hint: 1/0/0 2/0/0 3/00 4/0/0 M/0/0] [key type:0,vpc_id:0,proto:17,src:20190000000000000000000002000009,dst:20190000000000000000000002000001,sport:4634,dport:2560] [data role:0,session:529026,nhgroup:0,valid:1] Session data: 0000000000000000000000000000000000000000000000000000000000000000
[#/msb/hint: 1/0/0 2/0/0 3/00 4/0/0 M/0/0] [key type:0,vpc_id:0,proto:17,src:20190000000000000000000002000001,dst:20190000000000000000000002000009,sport:3504,dport:5608] [data role:0,session:1903789,nhgroup:0,valid:1] Session data: 0000000000000000000000000000000000000000000000000000000000000000
[#/msb/hint: 1/0/0 2/0/0 3/00 4/0/0 M/0/0] [key type:0,vpc_id:0,proto:17,src:20190000000000000000000002000001,dst:20190000000000000000000002000009,sport:8470,dport:706] [data role:0,session:593574,nhgroup:0,valid:1] Session data: 0000000000000000000000000000000000000000000000000000000000000000
```
### Display flow statistics
To display VPP-FTL statistics, below command can be used.
```
show flow statistics [detail] [thread <thread-id>] [ip4 | ip6]
```
With option - detail, statistics will be shown from all the VPP threads. Also option - thread can be used to get details for a particular thread.

Sample output of the command which shows summarised statistics across all VPP worker threads:
```
vpp# show flow statistics
IPv4 flow statistics summary:
Total number of IPv4 flow entries in hardware 8978800
Insert 16949144, Insert_fail_dupl 190, Insert_fail 0, Insert_fail_recirc 0
Remove 6, Remove_not_found 0, Remove_fail 0
Update 0, Update_fail 0
Get 0, Get_fail 0
Reserve 0, reserve_fail 0
Release 0, Release_fail 0
Tbl_insert 16949144, Tbl_remove 6, Tbl_read 136548359, Tbl_write 23224546
Tbl_lvl 0, Tbl_insert 10673752, Tbl_remove 4
Tbl_lvl 1, Tbl_insert 6275392, Tbl_remove 2
Tbl_lvl 2, Tbl_insert 0, Tbl_remove 0
Tbl_lvl 3, Tbl_insert 0, Tbl_remove 0
Tbl_lvl 4, Tbl_insert 0, Tbl_remove 0
Tbl_lvl 5, Tbl_insert 0, Tbl_remove 0
Tbl_lvl 6, Tbl_insert 0, Tbl_remove 0
Tbl_lvl 7, Tbl_insert 0, Tbl_remove 0

IPv6 flow statistics summary:
Total number of IPv6 flow entries in hardware 7798412
Insert 7798416, Insert_fail_dupl 3033091, Insert_fail 0, Insert_fail_recirc 0
Remove 0, Remove_not_found 0, Remove_fail 0
Update 0, Update_fail 0
Get 0, Get_fail 0
Reserve 0, reserve_fail 0
Release 0, Release_fail 0
Tbl_insert 7798416, Tbl_remove 0, Tbl_read 101588164, Tbl_write 10450106
Tbl_lvl 0, Tbl_insert 5146726, Tbl_remove 0
Tbl_lvl 1, Tbl_insert 2651690, Tbl_remove 0
Tbl_lvl 2, Tbl_insert 0, Tbl_remove 0
Tbl_lvl 3, Tbl_insert 0, Tbl_remove 0
Tbl_lvl 4, Tbl_insert 0, Tbl_remove 0
Tbl_lvl 5, Tbl_insert 0, Tbl_remove 0
Tbl_lvl 6, Tbl_insert 0, Tbl_remove 0
Tbl_lvl 7, Tbl_insert 0, Tbl_remove 0
```
### Clear all flows
Use below command to clear all flow entries from both IPv4 and IPv6 flow tables in hardware.
```
clear flow entries
```
### Clear flow statistics
Use below command to clear flow statistics. 
```
clear flow statistics [thread <thread-id>] [ip4 | ip6]
```
Option - thread provides flexibility to clear statistics for a particular thread. Also using ip4/ip6 filters, statistics of each addres-family can be cleared. Without any optional parameters, flow statistics will be cleared for all VPP worker threads and for both IPv4/IPv6.
Sample output for the command:
```
vpp# clear flow statistics
Clearing IPv4 flow statistcs for thread[0]
Clearing IPv6 flow statistcs for thread[0]
Clearing IPv4 flow statistcs for thread[1]
Clearing IPv6 flow statistcs for thread[1]
Clearing IPv4 flow statistcs for thread[2]
Clearing IPv6 flow statistcs for thread[2]
Clearing IPv4 flow statistcs for thread[3]
Clearing IPv6 flow statistcs for thread[3]
Successfully cleared flow statistics
```

