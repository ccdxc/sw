#!/bin/bash -e
SUDO=
#$SUDO vconfig add hntap_host0 103
$SUDO ifconfig hntap_host0 64.1.0.4 netmask 255.255.255.0 up
$SUDO route add 64.0.0.1 gw 64.1.0.4



$SUDO ifconfig hntap_net0 64.0.0.2 netmask 255.255.255.0 up
$SUDO route add 64.1.0.3 gw 64.0.0.2

