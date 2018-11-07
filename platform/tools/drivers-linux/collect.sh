#/bin/bash

#
# Collect driver debug materials.
#
# Synopsis: ./collect.sh [dest_dir]
#

COLLECT_DIR="${1:-collect}"

rm -rf "$COLLECT_DIR"
mkdir -p "$COLLECT_DIR"

{ # capture stderr

cp --parents -r \
    "/sys/kernel/debug/ionic" \
    "/sys/module/ionic" \
    "/sys/module/ionic_rdma" \
    "/proc/interrupts" \
    "/proc/cpuinfo" \
    "$COLLECT_DIR"

dmesg > "$COLLECT_DIR/dmesg.txt"

# As part of collect, warnings may have been printed to stderr, such as "file
# could not be opened for reading."  These are expected and mostly harmless, so
# don't worry the user about it.  Instead, save any warnings from the collect
# script in the collected materials themselves, just in case.
} 2> "$COLLECT_DIR/collect-stderr.txt"

echo "Driver debug materials have been collected in $COLLECT_DIR"
