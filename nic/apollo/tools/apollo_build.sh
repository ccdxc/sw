#!/bin/sh
set -x

NICDIR=`pwd`
copy_libs()
{
    mkdir -p build
    cd sdk/third-party/libs
    cp -r --parents -u ./* $NICDIR/build
    cd -
}

build_x86() {
    if [ ! -d  ./build ];then
        copy_libs
    fi
    make PIPELINE=apollo "${@:1}"
}

build_arm() {
    if [ ! -d  ./build ];then
        copy_libs
    fi
    make PIPELINE=apollo ARCH=aarch64 PLATFORM=hw "${@:1}"
}

build_fw() {
    if [ ! -d  ./build ];then
        copy_libs
    fi
    make PIPELINE=apollo ARCH=aarch64 PLATFORM=hw firmware "${@:1}"
}

print_usage() {
    echo "./apollo_build.sh <x64/a64/fw/clean> <extra make params>"
    echo "                   extra make params : V=1, -d etc"
}

clean() {
    rm -rf ./build
    rm -rf ../fake_root_target
}

if [ $# -lt 1 ];then
    print_usage
    exit
fi

if [ $1 == "x64" ];then
    build_x86 "${@:2}"
elif [ $1 == "a64" ];then
    build_arm "${@:2}"
elif [ $1 == "fw" ];then
    build_fw "${@:2}"
elif [ $1 == "clean" ];then
    clean
else
    print_usage
fi
