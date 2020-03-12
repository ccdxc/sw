#!/bin/bash

set -e

GIT_TAG=LMDB_0.9.25
INSTALL_DIR=/home/avinash/lmdb/install
AARCH64_GCC=/tool/toolchain/aarch64-1.1/bin/aarch64-linux-gnu-gcc

OPTIONS=sh
LONGOPTS=skip-clone,help

! PARSED=$(getopt --options=$OPTIONS --longoptions=$LONGOPTS --name "$0" -- "$@")
if [[ ${PIPESTATUS[0]} -ne 0 ]]; then
    exit 2
fi

eval set -- "$PARSED"

skip_clone=0

while true; do
    case "$1" in
        -s|--skip-clone)
            skip_clone=1
            shift
            ;;
        -h|--help)
            echo "Usage:"
            echo "$0 [--skip-clone]"
            exit 0
            ;;
        --)
            shift
            break
            ;;
        *)
            echo "error"
            exit 3
            ;;
    esac
done

echo "Git tag $GIT_TAG"
echo "Install dir $INSTALL_DIR"

if [[ "$skip_clone" == "0" ]]; then
    # clone git repo
    git clone https://github.com/openldap/openldap

    cd openldap

    # checkout tag
    git checkout $GIT_TAG
fi

cd libraries/liblmdb

# clean first
make clean

# x86_64 compilation
make

# x86_64 installation
make prefix=$INSTALL_DIR/x86_64 install

# clean before another make
make clean

# aarch64 compilation
make CC=$AARCH64_GCC

# aarch64 installation
make prefix=$INSTALL_DIR/aarch64 install
