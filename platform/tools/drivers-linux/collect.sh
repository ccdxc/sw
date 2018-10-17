#/bin/bash

#
# Collect driver debug materials.
#
# Synopsis: ./collect.sh [dest_dir]
#

COLLECT_DIR="${1:-collect}"
rm -r "$COLLECT_DIR"
cp -r "/sys/kernel/debug/ionic" "$COLLECT_DIR"
dmesg > "$COLLECT_DIR/dmesg.txt"

echo "Driver debug materials have been collected in $COLLECT_DIR"
