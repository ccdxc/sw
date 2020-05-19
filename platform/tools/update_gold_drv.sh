#!/bin/bash

PLATFORM_DIR=$PWD/../

ESX_DIR=$PLATFORM_DIR/hosttools/x86_64/esx/goldfw
LINUX_DIR=$PLATFORM_DIR/hosttools/x86_64/linux/goldfw
FREEBSD_DIR=$PLATFORM_DIR/hosttools/x86_64/freebsd/goldfw
WINDOWS_DIR=$PLATFORM_DIR/hosttools/x86_64/windows/goldfw

ESX_DRIVER=$PLATFORM_DIR/gen/drivers-esx-eth.tar.xz
LINUX_DRIVER=$PLATFORM_DIR/gen/drivers-linux-eth.tar.xz
FREEBSD_DRIVER=$PLATFORM_DIR/gen/drivers-freebsd-eth.tar.xz
WINDOWS_DRIVER=$PLATFORM_DIR/gen/drivers-windows.zip

remove_old()
{
    rm $1/old
}

update_latest()
{
    if [[ ! -f $2 ]]; then
        echo "file $2 not found"
        exit 1
    fi

    new_file=`readlink -f $2`
    cd $1
    prev_latest=`ls -l latest | awk -F'->' {'print $2'}`

    if [ $prev_latest = $3 ]; then
        echo "symlinks are already updated! No change is needed."
        return 0
    fi

    echo "Removing current old symlink"
    remove_old $1
    echo "Moving current latest to new old"
    ln -s $prev_latest old
    mkdir -p $3
    cp $new_file $3/.
    echo "Removing current latest symlink"
    rm latest
    echo "Creating new latest"
    ln -s $3 latest
    cd - > /dev/null
}

print_versions()
{
    echo ""
    cd $1
    echo "======== $1 ========"
    echo "latest => `ls -l latest | awk -F'->' {'print $2'}`"
    echo "old    => `ls -l old | awk -F'->' {'print $2'}`"
    cd - > /dev/null
    echo ""
}

if [ -z $1 ]; then
    echo "Usage: ./update_gold_drv.sh <fw version for goldfw>"
    exit 1
fi

echo "Checking Current Versions:"

mkdir -p $ESX_DIR
mkdir -p $LINUX_DIR
mkdir -p $FREEBSD_DIR
mkdir -p $WINDOWS_DIR

print_versions $ESX_DIR
print_versions $LINUX_DIR
print_versions $FREEBSD_DIR
print_versions $WINDOWS_DIR

echo "Updating the symlink for old and latest"
update_latest $ESX_DIR $ESX_DRIVER $1
update_latest $LINUX_DIR $LINUX_DRIVER $1
update_latest $FREEBSD_DIR $FREEBSD_DRIVER $1
update_latest $WINDOWS_DIR $WINDOWS_DRIVER $1

echo "Newer Versions installed are:"
print_versions $ESX_DIR
print_versions $LINUX_DIR
print_versions $FREEBSD_DIR
print_versions $WINDOWS_DIR

