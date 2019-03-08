#!/bin/bash

# Update version strings in drivers, penctl, etc.

if [ $# -ne 1 ] ; then
    echo "usage: $0 <new-version-number>"
    exit 1
fi

VER="$1"
TOP="$(readlink -f "$(dirname "$0")/..")"

# Update version string of penctl
echo "$VER" > "$TOP/penctl/version.txt"

# Update version strings of device drivers
CMD=(
sed -i "s/^\\(#define DR\\w*_VER\\w*\\s\\+\"\\).*\\(\"\\)\$/\1$VER\2/"
#"$TOP/platform/drivers/esxi/ionic_en/ionic.h"
"$TOP/platform/drivers/freebsd/usr/src/sys/dev/ionic/ionic_rdma/ionic_ibdev.c"
"$TOP/platform/drivers/freebsd/usr/src/sys/dev/ionic/ionic_eth/ionic.h"
"$TOP/platform/drivers/linux/eth/ionic/ionic.h"
#"$TOP/platform/drivers/linux/mnet/mnet_drv.h"
"$TOP/platform/drivers/linux/rdma/drv/ionic/ionic_ibdev.c"
#"$TOP/platform/drivers/pxe/ionic.h"
"$TOP/storage/offload/src/drv/linux/kernel/sonic.h"
)
"${CMD[@]}"
