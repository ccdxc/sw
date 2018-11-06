#! /bin/bash
killall memtun
/naples/memtun 1.0.0.1 > /naples/memtun.log 2>&1 &
pidof memtun
