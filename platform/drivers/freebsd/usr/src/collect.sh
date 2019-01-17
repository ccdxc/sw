#!/bin/sh
#
# Script to collect host-side debug materials.
#
# Synopsis:
#
# collect.sh [dir]
#
# Collect materials in directory named dir, which is 'collect' by default.
#

DIR="${1:-collect}"

mkdir -p "$DIR"
cd "$DIR"

# capture stderr to collect-err.txt
{

	# kernel log messages
	dmesg > dmesg.txt

	# linuxkpi module parameters
	sysctl compat.linuxkpi > sysctl-linuxkpi.txt

	# non-opaque values from sysctl
	sysctl dev.ionic > sysctl-ionic.txt

	# opaque values from sysctl as hexdump
	mkdir sysctl-opaque
	for ONAME in $(sysctl -t dev.ionic | sed -n -e 's/: opaque$//p') ; do
		sysctl -nb "$ONAME" | hd -v > "sysctl-opaque/$ONAME.txt"
	done

# capture stderr to collect-err.txt
} 2> collect-err.txt
