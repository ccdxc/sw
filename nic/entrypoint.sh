#!/bin/sh

dir=/usr/src/github.com/pensando/sw

echo 1>&2 syncing and building asset tools...

mkdir -p $dir
rsync -a /sw/build /sw/vendor ${dir}

cd $dir && go install ./build/... && cd /sw

exec $*
