#!/bin/bash -e

#sudo vconfig add hntap_host0 103
sudo ifconfig hntap_host0 64.1.0.4 netmask 255.255.255.0 up
sudo route add 64.0.0.1 gw 64.1.0.4



sudo ifconfig hntap_net0 64.0.0.2 netmask 255.255.255.0 up
sudo route add 64.1.0.3 gw 64.0.0.2

