#! /usr/local/bin/bash
killall memtun
/naples/memtun 1.0.0.1 > /naples/memtun.log 2>&1 &
sleep 3
ps -ax | grep memtun
ifconfig tun0 1.0.0.1 1.0.0.2
cd /naples/drivers/drivers-freebsd/ && kldload sys/modules/ionic/ionic.ko
