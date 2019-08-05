#! /bin/bash

TOOLS_DIR=`dirname $0`
ABS_TOOLS_DIR=`readlink -f $TOOLS_DIR`
NICDIR=`readlink -f $ABS_TOOLS_DIR/../../`

set -x
echo $NICDIR
OUTDIR='/tmp/'

create_doc() {
    type=$1
    rm -rf $OUTDIR/latex $OUTDIR/html
    doxygen $NICDIR/apollo/$type/Doxyfile > $OUTDIR/docout 2>&1
    if [ $type == 'api' ];then
        err=`cat $OUTDIR/docout | grep 'warning' | grep -v 'SDK_RET_OK' | grep -v '__PACK__'`
    else
        err=`cat $OUTDIR/docout | grep 'warning'`
    fi
    if [ ! -z $err ]; then
        echo "Error in $type documentation"
        cat $OUTDIR/docout
        exit 1
    fi
    if [ -d $OUTDIR/latex ];then
        cd $OUTDIR/latex
        make
        cd -
        if [ ! -f $OUTDIR/latex/refman.pdf ];then
            echo "Error in $type pdf generation"
            exit 1
        fi
        cp  $OUTDIR/latex/refman.pdf $NICDIR/apollo/doc/gen/pds_$type.pdf
    fi
}

mkdir -p $NICDIR/apollo/doc/gen
rm -rf $NICDIR/apollo/doc/gen/*
create_doc 'api'
create_doc 'test'
cd $NICDIR/apollo/doc/gen
tar -zcf pdsdoc.tar.gz pds*
cd -
exit 0
