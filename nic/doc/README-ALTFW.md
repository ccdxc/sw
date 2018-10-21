#!/bin/sh
#
# ./alt_uimage is a small-ish image that fits in the altfw partition
# and can be used to recover from a corrupted mainfw image.
# Copy all the files in this directory to Naples using memtun,
# and follow these steps to install the images into the altfw partition.
# Or simply use this README.altfw file as a script:
#     sh README.altfw
#
# Boot from the altfw partition by typing ^B at the uboot load step,
# then "bootalt".

# erase altfw partition
flash_erase /dev/mtd2 0 314

# install alt_dtb
dd if=alt_dtb of=/dev/mtd2

# install alt_uimage
dd if=alt_uimage of=/dev/mtd2 bs=64k seek=1
