#!/bin/bash

set -x
set -e

#RM="echo rm"
#MV="echo mv"
RM=/bin/rm
MV=/bin/mv

GRUBFILE=/run/initramfs/live/boot/grub2/grub.cfg
echo "venice_appl_GrubEntry.sh called with args $*"

while getopts ":oAdv:g:p:u:" arg; do
  case ${arg} in
    A) OP="add";;   # add a given version in the grubfile
    u) OP="doupgrade";   # Given a directory as its argument, do the upgrade of image (except reboot)
        SRCPATH=$OPTARG;;
    p) OP="preupgrade";   # Given a directory as its argument, do all the pre-upgrade check
        SRCPATH=$OPTARG;;
    d) OP="del";;   # delete a given version from the grubfile (as well as images)
    o) OP="only";; # keep only the currently specified default in the grubfile
    v) VERSION=$OPTARG;; # used for add and delete operations
    g) GRUBFILE=$OPTARG;;   # to override the default grub file
  esac
done

if [[ ! -w "${GRUBFILE}" || ! -f ${GRUBFILE} ]]
then
    echo ${GRUBFILE} is not writable
    exit 1
fi

if [[ "${OP}" == "del" && -z "$VERSION" ]]
then
    echo VERSION  must be specified with -d option
    exit 2
fi
if [[ "${OP}" == "add" && -z "$VERSION" ]]
then
    echo VERSION  must be specified with -A option
    exit 3
fi

if [[ -z "$OP" ]]
then
    echo Operation  must be specified
    exit 4
fi

function getBootUUID() {
    UUID=$(blkid -s UUID -t LABEL=PENBOOT -o export | grep UUID | head -n 1 | cut -f2 -d=)
    if [[ -z "$UUID" ]]
    then
        echo Boot partition not found or labelled correctly. Please check installation
        exit 5
    fi
}

function addMenuEntry() {
    local VER=$1
    local GRUBFILE=$2
    getBootUUID

    entryNum=$(grep menuentry ${GRUBFILE} | grep -n ${VER} | head -n 1 | cut -d: -f1)

    # add an entry of one does not exist already
    if [[ "${entryNum}" == "" ]]
    then

    cat >> ${GRUBFILE} <<END
menuentry ${VER} {
    linux16 /OS-${VER}/vmlinuz0 rw rd.fstab=0 root=live:UUID=${UUID} rd.live.dir=/OS-${VER} rd.live.squashimg=squashfs.img console=ttyS0 console=tty0 rd.live.image rd.luks=0 rd.md=0 rd.dm=0  enforcing=0 LANG=en_US.utf8 rd.writable.fsimg=1 pen.venice=OS-${VER}/venice.tgz pen.naples=OS-${VER}/naples_fw.tar
    initrd16 /OS-${VER}/initrd0.img
}
END
    fi

}

function setDefaultBootEntry() {
    local VER=$1
    local GRUBFILE=$2
    entryNum=$(grep menuentry ${GRUBFILE} | grep -n ${VER} | head -n 1 | cut -d: -f1)
    if [[ "${entryNum}" == "" ]]
    then
        echo Version must be specified.
        exit 6
    fi

    entryNum=$((entryNum - 1))

    #grub2-set-default "$entryNum" # we dont use env. so dont need this
    sed -i "s/default=.*/default=$entryNum/g" ${GRUBFILE}
}

function getCurBootVersion() {
    local GRUBFILE=$1
    entry=$(grep 'default=' ${GRUBFILE} | cut -d= -f2)
    if [[ -z "$entry" ]]
    then
        echo "Default entry not found in grub file. Exiting"
        exit 7
    fi
    entry=$((entry + 1))
    version=$(grep menuentry ${GRUBFILE}  | sed -n "${entry}p" | awk '{print $2}')
    if [ -z "$version" ]
    then
        echo "unable to determine the currently booted version in grub"
        exit 8
    fi
    echo ${version}
}

# set the default to entry #0
function setDefaultFirstEntry() {
    local GRUBFILE=$1
    sed -i "s/default=.*/default=0/g" ${GRUBFILE}
}

function doAddImage() {
    local VER=$1
    local GRUBFILE=$2
    if [[ ! -d /run/initramfs/live/OS-${VER} ]]
    then
        echo OS version ${VER} is not installed.
        exit 9
    fi
    addMenuEntry ${VER} ${GRUBFILE}
    setDefaultBootEntry ${VER} ${GRUBFILE}
}

function delMenuEntryVersion() {
    local VER=$1
    local GRUBFILE=$2
    if [[ -z "$(grep menuentry ${GRUBFILE} | grep ${VER})" ]]
    then
        echo Version ${VER} not found in file ${GRUBFILE}
        exit 10
    fi
    sed -i "/ ${VER} /,/}/d" ${GRUBFILE}
}

function doDelImage() {
    local VER=$1
    local GRUBFILE=$2

    # lets operate on a temp file first
    TMPFILE=`mktemp /tmp/veniceGrubTool.XXXXXX` || exit 20
    cp ${GRUBFILE} ${TMPFILE}

    curBootVersion=$(getCurBootVersion ${GRUBFILE})

    delMenuEntryVersion ${VER} ${TMPFILE}

    remainingEntries=$(grep menuentry ${TMPFILE} | wc -l)
    if [[ ${remainingEntries} -le 0 ]]
    then
        echo "Cant delete the last bootable image"
        exit 11
    fi

    if  [[ "$curBootVersion" == "$VER" ]]
    then
        setDefaultFirstEntry ${TMPFILE}
    else
        setDefaultBootEntry ${curBootVersion} ${TMPFILE}
    fi


    ${MV} -f ${TMPFILE} ${GRUBFILE}
    ${RM} -fr /run/initramfs/live/OS-${VER}
}

function keepOnlyCurrentDefaultImage() {
    curBootVersion=$(getCurBootVersion ${GRUBFILE})
    for v in $(grep menuentry ${GRUBFILE}  | grep -v ${curBootVersion} | awk '{print $2}')
    do
        echo doDelImage ${v} ${GRUBFILE}
        doDelImage ${v} ${GRUBFILE}
    done
}

function doPreUpgrade() {
    if [[ ! -d "${SRCPATH}" ]]
    then
        echo "Cant read directory ${SRCPATH}"
        exit 14
    fi
    cd ${SRCPATH}
    if [[ ! -f venice.tgz || ! -f naples_fw.tar || ! -f venice_appl_os.tgz || ! -f metadata.json ]]
    then
        echo "Cant find all the files required. Only see $(ls)"
        exit 15
    fi
    VER=$(eval echo $(cat metadata.json | jq '.Bundle.Version'))
    if [[ -z "$VER" ]]
    then
        echo "Cant find bundle version in metadata"
        exit 16
    fi

    tar zxvf venice_appl_os.tgz

    diskused=$(du -kxc initrd0.img naples_fw.tar squashfs.img vmlinuz0 venice.tgz | grep total| cut -f1)
    diskavail=$(df -kP /run/initramfs/live  | grep  /run/initramfs/live |  awk '{print $4};')
    if [[ -z "$diskavail" || -z "$diskused" ]]
    then
        echo "Unable to determine free space available or disk space used"
        exit 17
    fi

    if [[ ${diskavail} -lt $((diskused + 10)) ]]
    then
        echo "Not enough free disk space to install image"
        exit 18
    fi
    diskavailpercent=$(df -kP /run/initramfs/live  | grep  /run/initramfs/live |  awk '{print $5};' | sed 's/%$//')
    if [[ ${diskavailpercent} -lt 3 ]]
    then
        echo "Disk is more than 97% full. Please free up atleast 5% diskspace in the system before rollout can proceed."
        exit 20
    fi
}

function doDeleteOldDockerImages() {
    allowedVerList=$(grep menuentry ${GRUBFILE}  | awk '{print $2}')
    if [[ -z ${allowedVerList} ]]
    then
        echo "allowedImage list to boot should not be empty"
        exit 19
    fi

    declare -A verMap
    for i in ${allowedVerList}
    do
        verMap[$i]=1
    done

    for imageId in $(docker images -f 'label=org.label-schema.vendor=Pensando' --format '{{.ID}}' )
    do
        lab=$(docker inspect -f '{{index .Config.Labels "org.label-schema.version"}}' $imageId)

        # label found. Now check if this is in one of the allowed lists by looking in the map
        if [[ ! -z ${lab}  && -z ${verMap[$lab]:+_} ]]
        then
            docker rmi ${imageId} || :
        else
            :    #echo skip ${imageId} as ${lab} is needed
        fi
    done
    # delete any untagged images
    docker system prune -f
}

function doUpgrade() {
    DEST=/run/initramfs/live/OS-${VER}
    mkdir -p ${DEST}
    cd ${SRCPATH}
    tar zxvf venice_appl_os.tgz
    for i in initrd0.img naples_fw.tar squashfs.img vmlinuz0 venice.tgz
    do
        cp ${SRCPATH}/${i} ${DEST}
    done
    doAddImage ${VER} ${GRUBFILE}
    doDeleteOldDockerImages
    sync
}


case ${OP} in
"add") doAddImage ${VERSION} ${GRUBFILE};;
"del") doDelImage ${VERSION} ${GRUBFILE};;
"only") keepOnlyCurrentDefaultImage;;
"preupgrade") doPreUpgrade ;;
"doupgrade") doPreUpgrade; keepOnlyCurrentDefaultImage; doUpgrade;;
esac

exit 0
