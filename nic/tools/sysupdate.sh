#!/bin/sh
set -e

VERIFY=0
DRYRUN=0

while getopts "p:vd" opt; do
  case ${opt} in
    p)
        PKG=$OPTARG
        ;;
    v)
        VERIFY=1
        ;;
    d)
        DRYRUN=1
        ;;
    *) echo "Usage: $0 [-p] path [-v]erify [-d]ryrun"
      ;;
  esac
done

echo "Options: pkg $PKG verify $VERIFY dryrun $DRYRUN"

if [ -z $PKG ]; then
    echo "Specify valid package path"
    return 1
fi

if [ ! -f $PKG ]; then
    echo "No such file or directory - $PKG"
    return 1
fi

PKG_DIR=`mktemp -d -t sysupdate_XXXXXXXXX`
echo "===> Creating package directory $PKG_DIR"

finish() {
    echo "===> Removing package directory $PKG_DIR"
    rm -r $PKG_DIR
}
trap finish EXIT


echo "===> Unpacking firmware package"
tar xf $PKG -C $PKG_DIR

#
# Image Verification
#
MANIFEST=$PKG_DIR/MANIFEST

if [ ! -f $MANIFEST ]; then
    echo "Manifest file not found!"
    return -1
fi

UBOOT=$PKG_DIR/u-boot.bin
UIMG=$PKG_DIR/uImage
DTB=$PKG_DIR/naples-100.dtb

EXP_UBOOT_HASH=`cat $MANIFEST | jq '.firmware.boot.files.image.verify.hash' | tr -d \"`
EXP_UIMG_HASH=`cat $MANIFEST | jq '.firmware.main.files.kernel.verify.hash' | tr -d \"`
EXP_DTB_HASH=`cat $MANIFEST | jq '.firmware.main.files.device_tree.verify.hash' | tr -d \"`

echo "===> Verifying U-Boot Image"
if [ -f $UBOOT ]; then

    UBOOT_HASH=`md5sum $UBOOT | cut -d' ' -f1`

    echo "UBOOT_HASH = $UBOOT_HASH"
    echo "EXP_UBOOT_HASH = $EXP_UBOOT_HASH"

    if [ "$UBOOT_HASH" != "$EXP_UBOOT_HASH" ]; then
        echo "Uboot image verification failed!"
        return 1
    fi

    UBOOT_SZ=`stat -c%s $UBOOT`
    echo "UBOOT_SZ = $UBOOT_SZ"
fi

echo "===> Verifying DTB Image"
if [ -f $DTB ]; then
    DTB_HASH=`md5sum $DTB | cut -d' ' -f1`

    echo "DTB_HASH = $DTB_HASH"
    echo "EXP_DTB_HASH = $EXP_DTB_HASH"

    if [ "$DTB_HASH" != "$EXP_DTB_HASH" ]; then
        echo "DTB image verfication failed!"
        return 1
    fi

    DTB_SZ=`stat -c%s $DTB`
    echo "DTB_SZ = $DTB_SZ"
fi

echo "===> Verifying Kernel Image"
if [ -f $UIMG ]; then
    UIMG_HASH=`md5sum $UIMG | cut -d' ' -f1`

    echo "UIMG_HASH = $UIMG_HASH"
    echo "EXP_UIMG_HASH = $EXP_UIMG_HASH"

    if [ "$UIMG_HASH" != "$EXP_UIMG_HASH" ]; then
        echo "uImage verification failed!"
        return 1
    fi

    UIMG_SZ=`stat -c%s $UIMG`
    echo "UIMG_SZ = $UIMG_SZ"
fi

#
# Flash Images
#
if [ $VERIFY != 1 ]; then

    dev_info() {
        NAME=$1
        DEV=`cat /proc/mtd | grep "$NAME" | cut -d':' -f1`
        [ ! -z "$DEV" ] || return 1
        DEV_SZ=`cat /proc/mtd | grep "$NAME" | cut -d' ' -f2`
        DEV_BLKSZ=`cat /proc/mtd | grep "$NAME" | cut -d' ' -f3`
    }

    if [ -f $UBOOT ]; then
        dev_info uboot
        NUM_BLOCKS="$(( (($UBOOT_SZ + 0x$DEV_BLKSZ - 1) / 0x$DEV_BLKSZ) ))"
        ERASE_CMD="flash_erase /dev/$DEV 0 $NUM_BLOCKS"
        WRITE_CMD="dd if=$UBOOT of=/dev/$DEV"
        if [ $DRYRUN = 1 ]; then
            echo "$ERASE_CMD"
            echo "$WRITE_CMD"
        else
            echo "===> Erasing U-Boot Image"
            $ERASE_CMD
            echo "===> Writing U-Boot Image"
            $WRITE_CMD
        fi
    fi

    if [ -f $DTB ]; then
        dev_info mainfw
        ERASE_CMD="flash_erase /dev/$DEV 0 1"
        WRITE_CMD="dd if=$DTB of=/dev/$DEV"
        if [ $DRYRUN = 1 ]; then
            echo "$ERASE_CMD"
            echo "$WRITE_CMD"
        else
            echo "===> Erasing DTB Image"
            $ERASE_CMD
            echo "===> Writing DTB Image"
            $WRITE_CMD
        fi
    fi

    if [ -f $UIMG ]; then
        dev_info mainfw
        NUM_BLOCKS="$(( (($UIMG_SZ + 0x$DEV_BLKSZ - 1) / 0x$DEV_BLKSZ) ))"
        ERASE_CMD="flash_erase /dev/$DEV 65536 $NUM_BLOCKS"
        WRITE_CMD="dd if=$UIMG of=/dev/$DEV bs=64k seek=1"
        if [ $DRYRUN = 1 ]; then
            echo "$ERASE_CMD"
            echo "$WRITE_CMD"
        else
            echo "===> Erasing Kernel Image"
            $ERASE_CMD
            echo "===> Writing Kernel Image"
            $WRITE_CMD
        fi
    fi
fi