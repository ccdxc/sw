************************
***** CONNECTIONS ******
************************

VNC:
pen-hwlab-pc-01:5
passwd: hapsuser

# HAPS serial console:
telnet 192.168.65.227 7001

# Power Strip
http://192.168.65.101/  (admin/admin)
Click "Reboot" button for "Host PC"

# HAPS Host BMC
http://192.168.65.80 (ADMIN/ADMIN - yes ALL CAPS)
Remote Control tab, select Power Control
Reset Server


***********************
***** SETUP CAPRI *****
***********************

WINDOW: HAPS

# Don't pass "GO?"

WINDOW: TCL

# Load Config
confprosh
source /home/haps/haps/scripts/common/haps.tcl
cfg_clr
cfg_load top16_rev110
source /home/haps/haps/scripts/common/go.tcl
exit

# ASIC init
/home/neel/haps/haps.sh top16_rev110
source /home/neel/haps/init.tcl
exit

# Prepate CAPRI for bootup
confprosh
source /home/haps/haps/scripts/common/haps.tcl 

**********************
***** BOOT CAPRI *****
**********************

WINDOW: HAPS

# Pass "GO?" by typing 'y'

setenv verify no
bootmain

# Login with root/pen123

# Mount Flash
mount -t jffs2 -o ro /dev/mtdblock1 /mnt

************************
***** INSTALL APPS *****
************************

WINDOW: TCL

copyout /home/neel/haps/hal.tar

WINDOW: HAPS

/mnt/bin/copyin /hal.tar
cd /
tar -xf hal.tar --strip-components=1

**********************
***** START APPS *****
**********************

WINDOW: HAPS 

# Start screen
screen

# Start HAL
cp /nic/conf/hal_classic.ini /nic/conf/hal.ini
/nic/tools/start-hal-haps.sh

# Create New Window: Ctrl+a c

# Wait for HAL to become ready
tail -f /hal.log | grep gRPC

# Create New Window: Ctrl+a c

# Run HAL config script ... Wait for it to finish
export LD_LIBRARY_PATH=/nic/lib:$LD_LIBRARY_PATH
/nic/bin/hal_test

# Create New Window: Ctrl+a c

# Start NICMGR/PCIMGR ... Wait for it to print 'Polling'
export LD_LIBRARY_PATH=/platform/lib:/nic/lib:$LD_LIBRARY_PATH
/platform/bin/nicmgrd -m <haps_num> -p

***********************
***** BRINGUP MAC *****
***********************

WINDOW: TCL

# Bringup MAC 0
source /home/haps/haps/scripts/common/mxp.tcl
mxp_up 0

# Check link status a few times, The last byte of output should be '6d'.
# If link does not come up then, you need to go back to *SETUP CAPRI* step.

phy_regrd 0 0 1
temac_statrd 0 0 1

WINDOW: PEER

# Test MAC

# Login with root/docker
ssh root@192.168.68.103
~/net.sh

# Check if link is up
ip link show dev enp3s0f1

# Send an arp packet
arping -c1 -Ienp3s0f1 10.10.1.2 

WINDOW: TCL

# Make sure packet counters incremented
# If the counters do not increment then, you need to go back to *SETUP CAPRI* step.
temac_statrd 0 0 1

***********************
***** SETUP HOSTS *****
***********************

===== CONNECTIONS =====

HOST00
ssh root@192.168.65.95

HOST03
ssh root@192.168.67.95

PEER00
ssh root@192.168.64.103

=======================

WINDOW: PEER HOST

~/net.sh enp3s0f1 up

WINDOW: HOST

# Poweron Host

# login and reboot HAPS test host (root/docker)
reboot

# Check if devices are visible to the host
lspci -d 1dd8:

# You should see 'Ethernet Controller' devices
3b:00.0 PCI bridge: Device 1dd8:1000
3c:00.0 PCI bridge: Device 1dd8:1001
3d:00.0 Ethernet controller: Device 1dd8:1002

# Load driver
insmod ionic/ionic.ko
~/net.sh enp61s0 up

*******************************
***** DATAPATH MONITORING *****
*******************************

WINDOW: HAPS

/mnt/pensando/bin/capmon_poll.sh

# Copyout log files after test is complete
/mnt/pensando/bin/copyout /capmon.log

WINDOW: TCL

copyin /tmp/capmon.log

WINDOW: HAPS

/mnt/pensando/bin/copyout /capmon_v.log

WINDOW: TCL

copyin /tmp/capmon_v.log


*************************
***** PKTGEN NON-SG *****
*************************

WINDOW: PEER HOST

./net-next/samples/pktgen/pktgen_sample01_simple.sh -i enp3s0f1 -s 60 -m 00:02:00:02:01:03 -d 10.10.1.3 -v -x -n 100000000


WINDOW: HOST

./net-next/samples/pktgen/pktgen_sample01_simple.sh -i enp61s0 -s 60 -m 00:02:0a:0a:01:02 -d 10.10.1.2 -v -x -n 100000000

*********************
***** IPERF UDP *****
*********************

WINDOW: PEER HOST

iperf -u -s -i 5 2>&1 > server.log &
iperf -u -c 10.10.1.3 -i 1 -l 64 -t 28800 -b1G -e 2>&1 > client.log &

# Copyout server.log and client.log after test is completed

WINDOW: HOST

iperf -u -s -i 5 2>&1 > server.log &
iperf -u -c 10.10.1.2 -i 1 -l 64 -t 28800 -b1G -e 2>&1 > client.log &

# Copyout server.log and client.log after test is completed

*************************
***** HAPS CHEAT SHEET *****
*************************
https://docs.google.com/document/d/11atOGWkLxmu5qaLq1CzTrnp9oftib2wWzFgsVTWWxSg/edit#heading=h.ss00tdkauvy8

