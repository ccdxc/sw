#! /bin/bash -e

SCRIPT_DIR='diname $0'
LKL_DIR='../'
LKL_EXPORT_INC_DIR='../export/include/'

cd $LKL_DIR

X86_64=0
AARCH64=0
CLONE=0
BUILD=0
CLEAN=0
REPLACE=0
DELETE=0
NEWENTRYPOINT=0
HELP=0

if [ "$#" -eq "0" ]
then
    CLONE=1
    BUILD=1
    REPLACE=1
    X86_64=1
fi

for i in "$@"
do
case $i in
    -x|--x86_64)
    X86_64=1
    #shift # past argument=value
    ;;
    -a|--aarch64)
    AARCH64=1
    #shift # past argument=value
    ;;
    -b|--build)
    BUILD=1
    #shift # past argument=value
    ;;
    -c|--clone)
    CLONE=1
    #shift # past argument=value
    ;;
    -mc|--makeclean)
    CLEAN=1
    #shift # past argument=value
    ;;
    -r|--replace)
    REPLACE=1
    #shift # past argument=value
    ;;
    -d|--delete)
    DELETE=1
    #shift # past argument=value
    ;;
    -nep|--newentrypoint)
    NEWENTRYPOINT=1
    #shift # past argument=value
    ;;
    -h|--help)
    HELP=1
    #shift # past argument=value
    ;;
    *)
            # unknown option
    ;;
esac
done

if [ "$X86_64" -eq "1" ]
then
    echo "Building for x86_64"
    LKL_EXPORT_BIN_DIR="../export/x86_64/bin/"
fi

if [ "$AARCH64" -eq "1" ]
then
    echo "Building for aarch64"
    LKL_EXPORT_BIN_DIR="../export/aarch64/bin/"
fi

if [ "$CLONE" -eq "1" ]
then
    echo "Cloning..."
    echo "Deleting ../linux/ ..."
    rm -rf linux/
    echo "Creating ../linux/ ..."
    mkdir linux
    cd linux
    echo "Cloning into linux/ ..."
    git clone git@github.com:pensando/lkl.git .
    cd ..
fi

if [ "$BUILD" -eq "1" ]
then
    echo "Building..."
    cd linux
    echo "Running make clean..."
    make -C tools/lkl clean
    echo "Running make..."
    make -C tools/lkl -j40 | tee lkl_build.log
    cd ..
fi

if [ "$REPLACE" -eq "1" ]
then
    echo "Replacing..."
    cd linux
    rm -rf $LKL_EXPORT_INC_DIR/*
    echo "Deleted $LKL_EXPORT_INC_DIR/* ..."
    rm -rf $LKL_EXPORT_BIN_DIR/*
    echo "Deleted $LKL_EXPORT_BIN_DIR/* ..."
    cp -rf tools/lkl/include/* $LKL_EXPORT_INC_DIR
    echo "Copied lkl/include to  $LKL_EXPORT_INC_DIR/* ..."
    cp tools/lkl/lib/liblkl.so $LKL_EXPORT_BIN_DIR
    echo "Copied liblkl.so to  $LKL_EXPORT_BIN_DIR/* ..."
    cd ..
fi

if [ "$CLEAN" -eq "1" ]
then
    echo "Make Clean..."
    cd linux
    make -C tools/lkl clean
    cd ..
fi


if [ "$DELETE" -eq "1" ]
then
    echo "Deleting linux..."
    rm -rf linux/
fi

if [ "$NEWENTRYPOINT" -eq "1" ]
then
    echo "Deleting vmlinux, lkl.o..."
    rm -rf linux/vmlinux
    rm -rf linux/lkl.o
fi


if [ "$HELP" -eq "1" ]
then
    echo "lkl.sh -b|--build      : Build LKL code"
    echo "lkl.sh -c|--clone      : Clone LKL code"
    echo "lkl.sh -mc|--makeclean : Make Clean LKL code"
    echo "lkl.sh -r|--replace    : Replace export/ content from lkl tree"
    echo "lkl.sh -d|--delete     : Delete lkl tree"
    echo "lkl.sh -nep|--newentrypoint     : Delete lkl.o vmlinux"
fi
