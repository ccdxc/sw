#!/bin/sh
set -x

NICDIR=`pwd`
copy_libs()
{
    if [ ! -d  ./build/$1/apollo ];then
        mkdir -p build/$1
        cd sdk/third-party/libs/$1
        cp -r --parents -u ./* $NICDIR/build/$1
        cd -
    fi
}

build_x86() {
    copy_libs x86_64
    make PIPELINE=apollo "${@:1}"
}

build_arm() {
    copy_libs aarch64
    make PIPELINE=apollo ARCH=aarch64 PLATFORM=hw PERF=1 "${@:1}"
}

build_fw() {
    copy_libs aarch64
    make PIPELINE=apollo ARCH=aarch64 PLATFORM=hw PERF=1 firmware  "${@:1}"
}

print_usage() {
    echo "./apollo_build.sh <x86_64/aarch64/firmware/clean> <extra make params>"
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

if [ $1 == "x86_64" ];then
    build_x86 "${@:2}"
elif [ $1 == "aarch64" ];then
    build_arm "${@:2}"
elif [ $1 == "firmware" ];then
    build_fw "${@:2}"
elif [ $1 == "clean" ];then
    clean
else
    print_usage
fi
