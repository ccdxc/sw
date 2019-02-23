# Capmon 
capmon is a monitoring tool to check various counters across all pipelines.


## Usage 
```
# capmon --help
usage: capmon -v[erbose] -r[eset] -q[ueues] -p[cie] -b[wmon] -s[pps] -l[lif] -t[qtype] -i[qid] -R[ring] -p[poll]<interval>

Example: capmon -q -l 1003:1005 -t 0 -i 0:5 -R 1 -p 100
```

This is the sample non-verbose output for capmon
```
# capmon
== PCIe ==
 Target
  pending [0]=0
 Initiator
  wr_pend=0 rd_pend=0
  read latency (clks) >0=0.00% >250=100.00% >500=0.00% >1000=0.00%
 Port 0
  rx_stl=0.00% tx_stl=0.00%
== Doorbell ==
 Host=5611 Local=8914095 Sched=3264787
== TX Scheduler ==
 Set=190198 Clear=3074589 PB XOFF: 0% 0% 0% 0% 0% 0% 0% 0% 0% 0% 0% 0% 0% 0% 0% 0%
 PHVs:
== TXDMA ==
 NPV: phv=8767714 pb_pbus=8767714 pr_pbus=0 sw=0 phv_drop=0 recirc=0
 S0: (util/xoff/idle) in=  0/  0/100 stg=  0/  0/100 out=  0/  0/100 TE= 0 HBM_lat=    0 m0=  0% m1=  0% m2=  0% m3=  0%
 S1: (util/xoff/idle) in=  0/  0/100 stg=  0/  0/100 out=  0/  0/100 TE= 0 HBM_lat=   72 m0=  0% m1=  0% m2=  0% m3=  0%
 S2: (util/xoff/idle) in=  0/  0/100 stg=  0/  0/100 out=  0/  0/100 TE= 0 HBM_lat=    0 m0=  0% m1=  0% m2=  0% m3=  0%
 S3: (util/xoff/idle) in=  0/  0/100 stg=  0/  0/100 out=  0/  0/100 TE= 0 HBM_lat=    0 m0=  0% m1=  0% m2=  0% m3=  0%
 S4: (util/xoff/idle) in=  0/  0/100 stg=  0/  0/100 out=  0/  0/100 TE= 0 HBM_lat=    0 m0=  0% m1=  0% m2=  0% m3=  0%
 S5: (util/xoff/idle) in=  0/  0/100 stg=  0/  0/100 out=  0/  0/100 TE= 0 SRM_lat=   22 m0=  0% m1=  0% m2=  0% m3=  0%
 S6: (util/xoff/idle) in=  0/  0/100 stg=  0/  0/100 out=  0/  0/100 TE= 0 HBM_lat=    0 m0=  0% m1=  0% m2=  0% m3=  0%
 S7: (util/xoff/idle) in=  0/  0/100 stg=  0/  0/100 out=  0/  0/100 TE= 0 HBM_lat=    0 m0=  0% m1=  0% m2=  0% m3=  0%
 TxDMA: phv=8767714 pkt=290 drop=8578028(97%) err=0 recirc=0 resub=0 in_flight=0
       AXI reads=0 writes=0
       FIFO (empty%/full%) rd=0/0 wr=0/0 pkt=0/0
== RXDMA ==
 PSP: phv=16 pb_pbus=0 pr_pbus=16 sw=16 phv_drop=0 recirc=0
 S0: (util/xoff/idle) in=  0/  0/100 stg=  0/  0/100 out=  0/  0/100 TE= 0 HBM_lat=   26 m0=  0% m1=  0% m2=  0% m3=  0%
 S1: (util/xoff/idle) in=  0/  0/100 stg=  0/  0/100 out=  0/  0/100 TE= 0 HBM_lat=   75 m0=  0% m1=  0% m2=  0% m3=  0%
 S2: (util/xoff/idle) in=  0/  0/100 stg=  0/  0/100 out=  0/  0/100 TE= 0 HBM_lat=   24 m0=  0% m1=  0% m2=  0% m3=  0%
 S3: (util/xoff/idle) in=  0/  0/100 stg=  0/  0/100 out=  0/  0/100 TE= 0 HBM_lat=   76 m0=  0% m1=  0% m2=  0% m3=  0%
 S4: (util/xoff/idle) in=  0/  0/100 stg=  0/  0/100 out=  0/  0/100 TE= 0 HBM_lat=    0 m0=  0% m1=  0% m2=  0% m3=  0%
 S5: (util/xoff/idle) in=  0/  0/100 stg=  0/  0/100 out=  0/  0/100 TE= 0 HBM_lat=    0 m0=  0% m1=  0% m2=  0% m3=  0%
 S6: (util/xoff/idle) in=  0/  0/100 stg=  0/  0/100 out=  0/  0/100 TE= 0 HBM_lat=    0 m0=  0% m1=  0% m2=  0% m3=  0%
 S7: (util/xoff/idle) in=  0/  0/100 stg=  0/  0/100 out=  0/  0/100 TE= 0 HBM_lat=    0 m0=  0% m1=  0% m2=  0% m3=  0%
 RxDMA: phv=16 pkt=16 drop=0(0%) err=0 recirc=0 resub=0 in_flight=0
       AXI reads=0 writes=0
       FIFO (empty%/full%) rd=100/0 wr=100/0 pkt=100/0 ff_depth=0
       XOFF hostq=0 pkt=0 phv=0 phv_xoff=0% pb_xoff=0% host_xoff=0%
== P4IG ==
 Parser: pkt_from_pb=513 phv_to_s0=513 pkt_to_dp=513
 S0: (util/xoff/idle) in=  0/  0/100 stg=  0/  0/100 out=  0/  0/100 TE= 0 SRM_lat=   20 m0=  0% m1=  0% m2=  0% m3=  0%
 S1: (util/xoff/idle) in=  0/  0/100 stg=  0/  0/100 out=  0/  0/100 TE= 0 SRM_lat=   20 m0=  0% m1=  0% m2=  0% m3=  0%
 S2: (util/xoff/idle) in=  0/  0/100 stg=  0/  0/100 out=  0/  0/100 TE= 0 SRM_lat=   20 m0=  0% m1=  0% m2=  0% m3=  0%
 S3: (util/xoff/idle) in=  0/  0/100 stg=  0/  0/100 out=  0/  0/100 TE= 0 HBM_lat=   24 m0=  0% m1=  0% m2=  0% m3=  0%
 S4: (util/xoff/idle) in=  0/  0/100 stg=  0/  0/100 out=  0/  0/100 TE= 0 SRM_lat=   20 m0=  0% m1=  0% m2=  0% m3=  0%
 S5: (util/xoff/idle) in=  0/  0/100 stg=  0/  0/100 out=  0/  0/100 TE= 0 SRM_lat=   20 m0=  0% m1=  0% m2=  0% m3=  0%
== P4EG ==
 Parser: pkt_from_pb=525 phv_to_s0=525 pkt_to_dp=525
 S0: (util/xoff/idle) in=  0/  0/100 stg=  0/  0/100 out=  0/  0/100 TE= 0 SRM_lat=   20 m0=  0% m1=  0% m2=  0% m3=  0%
 S1: (util/xoff/idle) in=  0/  0/100 stg=  0/  0/100 out=  0/  0/100 TE= 0 SRM_lat=   20 m0=  0% m1=  0% m2=  0% m3=  0%
 S2: (util/xoff/idle) in=  0/  0/100 stg=  0/  0/100 out=  0/  0/100 TE= 0 HBM_lat=    0 m0=  0% m1=  0% m2=  0% m3=  0%
 S3: (util/xoff/idle) in=  0/  0/100 stg=  0/  0/100 out=  0/  0/100 TE= 0 SRM_lat=    0 m0=  0% m1=  0% m2=  0% m3=  0%
 S4: (util/xoff/idle) in=  0/  0/100 stg=  0/  0/100 out=  0/  0/100 TE= 0 SRM_lat=   20 m0=  0% m1=  0% m2=  0% m3=  0%
 S5: (util/xoff/idle) in=  0/  0/100 stg=  0/  0/100 out=  0/  0/100 TE= 0 SRM_lat=   20 m0=  0% m1=  0% m2=  0% m3=  0%

```

Each of the above sections are explained in detail below.

PCIe:
This section captures target and initiator pending writes and reads.
The counts of packets at various latencies are also displayed.

```
 Target
  pending [0]=0
 Initiator
  wr_pend=0 rd_pend=0
```
Number of outstanding PCIe reads/writes.

```
  read latency (clks) >0=0.00% >250=100.00% >500=0.00% >1000=0.00%
```
PCIe read latency histogram.

```
 Port 0
  rx_stl=0.00% tx_stl=0.00%
```
PCIe access stalls.



```
Doorbell
== Doorbell ==
 Host=5611 Local=8914095 Sched=3264787
```
Host: Number of doorbell writes generated by the host

Local: Number of doorbell writes generated locally by either the Data Plane or ARM

Sched: Number of scheduler events generated by the Doorbell engine

```
TX Scheduler
 Set=190198 Clear=3074589 PB XOFF: 0% 0% 0% 0% 0% 0% 0% 0% 0% 0% 0% 0% 0% 0% 0% 0%
```
Set: Number of set events received by the scheduler (from Doorbell machine)

Clear: Number of clear events received by the scheduler (from Doorbell machine)

PB XOFF: Per PktQueue back-pressure from the Packet Buffer


```
== P4IG ==
 Parser: pkt_from_pb=513 phv_to_s0=513 pkt_to_dp=513
 S0: (util/xoff/idle) in=  0/  0/100 stg=  0/  0/100 out=  0/  0/100 TE= 0 SRM_lat=   20 m0=  0% m1=  0% m2=  0% m3=  0%
```
Pipeline
The packet counts from each MPU in each stage from each pipeline is displayed
here.

## License


## Acknowledgments


## TODO items 
    (this section will go away eventually)
    1 rename to asicmon
    2 have top like concise UI for capmon
