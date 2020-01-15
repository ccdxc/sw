#!/bin/sh
#
# Script for installing freebsd libraries and tools
#
# Run AFTER build.sh.
#

cd contrib/ofed/libionic
make install || exit
cd -

if ! grep -sq LD_PRELOAD ~/.bash_login ; then
    echo "export LD_PRELOAD=/usr/lib/libionic.so.1" >> ~/.bash_login
fi
if ! grep -sq LD_PRELOAD ~/.login ; then
    echo "LD_PRELOAD=/usr/lib/libionic.so.1" >> ~/.login
fi

cd perftest
make install || exit
cd -

cd qperf
make install || exit
cd -

cp show_gid /usr/local/bin/
