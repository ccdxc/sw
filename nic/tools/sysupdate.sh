#!/bin/sh

# Compatibility wrapper around fwupdate

set -e

PKG=""
ACTION="-i all"
INSTALL=1

while getopts "p:vd" opt; do
  case ${opt} in
    p)
        PKG=$OPTARG
        ;;
    v)
        ACTION=-v
        INSTALL=0
        ;;
    d)
        ACTION="-i all -D"
        INSTALL=0
        ;;
    *) echo "Usage: $0 [-p] path [-v]erify [-d]ryrun"
      ;;
  esac
done

if [ -z $PKG ]; then
    echo "Specify valid package path"
    return 1
fi

if [ ! -f $PKG ]; then
    echo "No such file or directory - $PKG"
    return 1
fi

/nic/tools/fwupdate -p $PKG $ACTION
if [ $INSTALL -eq 1 ]; then
    # set active image
    /nic/tools/fwupdate -s altfw
fi
