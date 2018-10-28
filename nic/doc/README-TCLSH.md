# To get tclsh into NAPLES
ssh to naples or connect via console:

1.  scp -o StrictHostKeyChecking=no 1.1.1.1:/vol/asic_dump/kinjal/asic5/ip/cosim/diag/nic.tar.gz  /tmp/
2.  mkdir /tmp/asic_tclsh
3.  mv /tmp/nic.tar.gz /tmp/asic_tclsh/
4.  cd /tmp/asic_tclsh
5.  gunzip nic.tar.gz
6.  tar xf nic.tar
7.  cd nic/fake_root_target/nic/
8.  export ASIC_LIB_BUNDLE=`pwd`
9.  export ASIC_SRC=$ASIC_LIB_BUNDLE/asic_src
10. cd asic_lib/
11. source ./source_env_path
12. ./diag.exe

<tclsh starts>

You can run cmds like below:

% source $::env(ASIC_SRC)/ip/cosim/tclsh/.tclrc.diag
% cap_reconcile_pkt_counts

Usefule commands:

To disable spurious interrupts
==============================
cap_top_intr_enable 0 none
cap_top_disable_spurious_intr cap0
cap_top_intr_check 0 0 none

To clear all interrupts
=======================
cap_top_intr_clear 0 none

To check all interrupts
=======================
cap_top_intr_check 0 1 none

To enable pause on Capri
========================
sknobs_close
sknobs_get_value abc 0
sknobs_set_string cap0/top/mac_cfg/mx0/mode mode_1x100g
cap_mx_set_pause 0 0 0xff 0xff 0xff 0xff <port_num>

(port_num: 1 and 5 are the two uplink ports)

To Dump PB counters:
====================
% cap_pb_dump_cntrs 0 0  1

To Dump top counters
========================
% cap_top_dump_cntrs 0

To Dump MAC MIB counters
========================
% cap_mx_dump_mibs  0

To Dump Capmon
========================
% call_capmon none


