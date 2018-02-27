#! /bin/bash -e

SCRIPT_DIR='diname $0'
ARCH=`arch`
OPENSSL_DIR='../'
OPENSSL_EXPORT_INC_DIR='../include'
OPENSSL_EXPORT_BIN_DIR="../$ARCH/bin"
OPENSSL_EXPORT_LIB_DIR="../$ARCH/lib"

cd $OPENSSL_DIR

CLONE=0
BUILD=0
CLEAN=0
REPLACE=0
DELETE=0
HELP=0

if [ "$#" -eq "0" ]
then
    HELP=1
fi

for i in "$@"
do
case $i in
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
    -h|--help)
    HELP=1
    #shift # past argument=value
    ;;
    *)
            # unknown option
    ;;
esac
done

if [ "$CLONE" -eq "1" ]
then
    echo "Cloning..."
    echo "Deleting ../tree/ ..."
    rm -rf tree
    echo "Creating ../tree/ ..."
    mkdir tree
    cd tree
    echo "Cloning into tree/ ..."
    #git clone https://github.com/pensando/openssl.git .
    git clone https://github.com/openssl/openssl.git .
    git checkout tags/OpenSSL_1_1_0e -b 110e
    echo "Applying patches..."
    # apply patches
    git apply < ../patches/pen_01.diff
    cd ..
fi

if [ "$BUILD" -eq "1" ]
then
    echo "Building..."
    cd tree
    cp ../scripts/build_pen.sh .
    ./build_pen.sh
    cd ..
fi

if [ "$REPLACE" -eq "1" ]
then
    echo "Replacing..."
    cd tree
    rm -rf ../lib
    rm -rf ../bin
    rm -rf $OPENSSL_EXPORT_INC_DIR/*
    echo "Deleted $OPENSSL_EXPORT_INC_DIR/* ..."
    rm -rf $OPENSSL_EXPORT_BIN_DIR/*
    echo "Deleted $OPENSSL_EXPORT_BIN_DIR/* ..."
    rm -rf $OPENSSL_EXPORT_LIB_DIR/*
    echo "Deleted $OPENSSL_EXPORT_LIB_DIR/* ..."
    cp -rf export/include/* $OPENSSL_EXPORT_INC_DIR
    echo "Copied openssl/include to  $OPENSSL_EXPORT_INC_DIR/* ..."
    cp -rf export/lib/* $OPENSSL_EXPORT_LIB_DIR
    echo "Copied lib to  $OPENSSL_EXPORT_LIB_DIR/* ..."
    cp -rf export/bin/* $OPENSSL_EXPORT_BIN_DIR
    echo "Copied bin to  $OPENSSL_EXPORT_BIN_DIR/* ..."
    cd ..
fi

if [ "$CLEAN" -eq "1" ]
then
    echo "Make Clean..."
    cd tree
    make clean
    cd ..
fi


if [ "$DELETE" -eq "1" ]
then
    echo "Deleting tree..."
    rm -rf tree
fi

if [ "$HELP" -eq "1" ]
then
    echo "openssl.sh -b|--build      : Build OPENSSL code"
    echo "openssl.sh -c|--clone      : Clone OPENSSL code"
    echo "openssl.sh -mc|--makeclean : Make Clean OPENSSL code"
    echo "openssl.sh -r|--replace    : Replace export/ content from openssl tree"
    echo "openssl.sh -d|--delete     : Delete openssl tree"
fi
