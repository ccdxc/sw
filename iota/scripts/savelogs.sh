#! /bin/bash
IOTADIR=$1/iota/
TARBALL=$IOTADIR/iota_sanity_logs.tar.gz
rm -f $TARBALL
tar czf $TARBALL -P --ignore-failed-read \
        $IOTADIR/logs $IOTADIR/*.log
