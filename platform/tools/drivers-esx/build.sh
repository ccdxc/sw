#!/bin/sh

set -o pipefail
set -ex

DIR=$(dirname "$0")
DIR=$(readlink -f "$DIR")

echo $PWD

if  [[ $1 = "--install" ]]; then
    #Bourne shell does not have arrays, crude way to find if more than one vib exists
    i=0
    for FILENAME in `(find drivers/esxi/ionic_en/build/vib/ -name \*.vib)`
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


    vib=`(find drivers/esxi/ionic_en/build/vib/ -name \*.vib)`
    echo "Installing Ionic VIB echo $vib"

    set +e
    #Ignore error as ionic may not be installed
    esxcli software vib remove -n=ionic-en -f
    set -e

    cp $vib /tmp

    vibfile=$(basename "$vib")
    esxcli software vib install -v=/tmp/$vibfile -f

    echo "Installation of VIB successful..."
else
    if [ $1 -eq 67 ]
    then
        cd drivers/esxi/ionic_en/ && cp Makefile_6.7 Makefile && make
        mkdir -p .build/package/devel/linux32/release/nativeddk/NativeDDK/rpm/ionic_en/output/current/vib20/ionic-en && mv .build/package/devel/linux32/release/nativeddk/NativeDDK/rpm/ionic_en/output/current/*.vib .build/package/devel/linux32/release/nativeddk/NativeDDK/rpm/ionic_en/output/current/vib20/ionic-en
        /opt/vmware/vibtools-6.7.0-8169922/bin/vibpublish -g build/bundle/metadata.zip -e build/bundle/offline-bundle.zip  -t ESXi,6.7.0 -v .build/package/devel/linux32/release/nativeddk/NativeDDK/rpm/ionic_en/output/current -b build/bulletin.xml -n Pensando Systems --vendor-code=PEN  --force
        rm -rf .build
    else
        cd drivers/esxi/ionic_en/ && cp Makefile_6.5 Makefile && make
        mkdir -p .build/package/devel/linux32/release/nativeddk/NativeDDK/rpm/ionic_en/output/current/vib20/ionic-en && mv .build/package/devel/linux32/release/nativeddk/NativeDDK/rpm/ionic_en/output/current/*.vib .build/package/devel/linux32/release/nativeddk/NativeDDK/rpm/ionic_en/output/current/vib20/ionic-en
        /opt/vmware/vibtools-6.5.0-4598673/bin/vibpublish -g build/bundle/metadata.zip -e build/bundle/offline-bundle.zip  -t ESXi,6.5.0 -v .build/package/devel/linux32/release/nativeddk/NativeDDK/rpm/ionic_en/output/current -b build/bulletin.xml -n Pensando Systems --vendor-code=PEN  --force
        rm -rf .build
    fi
    echo "Building drivers successfull...."
fi
