#!/bin/bash

#set -x
set -e

#RM="echo rm"
#MV="echo mv"
RM=/bin/rm
MV=/bin/mv

GRUBFILE=/boot/grub2/grub.cfg

while getopts ":adv:g:" arg; do
  case $arg in
    a) OP="add";;
    d) OP="del";;
    v) VERSION=$OPTARG;;
    g) GRUBFILE=$OPTARG;;
  esac
done

if [ ! -w "$GRUBFILE" -o ! -f $GRUBFILE ]
then
    echo $GRUBFILE is not writable
    exit 1
fi

if [ -z "$VERSION" ]
then
    echo VERSION  must be specified with -v option
    exit 1
fi

if [ -z "$OP" ]
then
    echo Operation  must be specified - Either -a or -d option must be specified
    exit 1
fi

function getBootUUID() {
    UUID=$(blkid -s UUID -t LABEL=PENBOOT -o export | grep UUID | head -n 1 | cut -f2 -d=)
    if [ -z "$UUID" ]
    then
        echo Boot partition not found or labelled correctly. Please check installation
        exit 3
    fi
}

function addMenuEntry() {
    local VER=$1
    local GRUBFILE=$2
    getBootUUID
    cat >> ${GRUBFILE} <<END
menuentry $VER {
    linux16 /OS-${VER}/vmlinuz0 rw rd.fstab=0 root=live:UUID=${UUID} rd.live.dir=/OS-${VER} rd.live.squashimg=squashfs.img console=ttyS0 console=tty0 rd.live.image rd.luks=0 rd.md=0 rd.dm=0  enforcing=0 LANG=en_US.utf8 rd.writable.fsimg=1 pen.venice=OS-${VER}/venice.tgz pen.naples=OS-${VER}/naples_fw.tar
    initrd16 /OS-${VER}/initrd0.img
}

END
}

function setDefaultBootEntry() {
    local VER=$1
    local GRUBFILE=$2
    entryNum=$(grep menuentry ${GRUBFILE} | grep -n ${VER} | head -n 1 | cut -d: -f1)
    if [ "${entryNum}" == "" ]
    then
        echo Version must be specified.
        exit 1
    fi

    entryNum=$((entryNum - 1))

    #grub2-set-default "$entryNum" # we dont use env. so dont need this
    sed -i "s/default=.*/default=$entryNum/g" ${GRUBFILE}
}

function getCurBootVersion() {
    local VER=$1
    local GRUBFILE=$2
    entry=$(grep 'default=' ${GRUBFILE} | cut -d= -f2)
    if [ -z "$entry" ]
    then
        echo "Default entry not found in grub file. Exiting"
        exit 12
    fi
    entry=$((entry + 1))
    version=$(grep menuentry ${GRUBFILE}  | sed -n "${entry}p" | awk '{print $2}')
    if [ -z "$version" ]
    then
        echo "unable to determine the currently booted version in grub"
        exit 13
    fi
    echo $version
}

# set the default to entry #0
function setDefaultFirstEntry() {
    local GRUBFILE=$1
    sed -i "s/default=.*/default=0/g" ${GRUBFILE}
}

function doAddImage() {
    local VER=$1
    local GRUBFILE=$2
    if [ ! -d /run/initramfs/live/OS-${VER} ]
    then
        echo OS version $VERis not installed.
        exit 2
    fi
    addMenuEntry $VER $GRUBFILE
    setDefaultBootEntry $VER $GRUBFILE
}

function delMenuEntryVersion() {
    local VER=$1
    local GRUBFILE=$2
    if [ -z "$(grep menuentry $GRUBFILE | grep $VER)" ]
    then
        echo Version $VER not found in file $GRUBFILE
        exit 14
    fi
    sed -i "/${VER}/,/}/d" $GRUBFILE
}

function doDelImage() {
    local VER=$1
    local GRUBFILE=$2

    # lets operate on a temp file first
    TMPFILE=`mktemp /tmp/veniceGrubTool.XXXXXX` || exit 10
    cp $GRUBFILE $TMPFILE

    curBootVersion=$(getCurBootVersion $VER $GRUBFILE)

    delMenuEntryVersion $VER $TMPFILE

    remainingEntries=$(grep menuentry ${TMPFILE} | wc -l)
    if [ $remainingEntries -le 0 ]
    then
        echo "Cant delete the last bootable image"
        exit 11
    fi

    if  [ "$curBootVersion" == "$VER" ]
    then
        setDefaultFirstEntry ${TMPFILE}
    else
        setDefaultBootEntry $curBootVersion ${TMPFILE}
    fi


    $MV -f ${TMPFILE} ${GRUBFILE}
    $RM -fr /run/initramfs/live/OS-${VER}
}


case $OP in
"add") doAddImage $VERSION $GRUBFILE;;
"del") doDelImage $VERSION $GRUBFILE;;
esac

