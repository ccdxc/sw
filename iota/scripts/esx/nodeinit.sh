#!/bin/sh

set -o pipefail
set -ex

DIR=$(dirname "$0")
DIR=$(readlink -f "$DIR")

echo $PWD

if  [[ $1 = "--install" ]]; then
    #Bourne shell does not have arrays, crude way to find if more than one vib exists
    i=0
    for FILENAME in `(find /naples -name \*.vib)`
    do
        i=$((i + 1))
        if [ $i -gt 1 ]; then
            echo "More than one vib files found"
            exit 1
        fi
    done


    if [ $i -eq 0 ]; then
        echo "VIB files not found"
        exit 1
    fi


    vib=`(find /naples -name \*.vib)`
    echo "Installing Ionic VIB echo $vib"

    set +e
    #Ignore error as ionic may not be installed
    esxcli software vib remove -n=ionic-en -f
    set -e

    cp $vib /tmp

    vibfile=$(basename "$vib")
    esxcli software vib install -v=/tmp/$vibfile -f

    echo "Installation of VIB successful..."
fi
