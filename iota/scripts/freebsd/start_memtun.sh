#! /usr/local/bin/bash
killall memtun
/naples/memtun 1.0.0.1 > /naples/memtun.log 2>&1 &
pidof memtun
sleep 3
ifconfig tun0 1.0.0.1 1.0.0.2
