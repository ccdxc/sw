#!/bin/sh

warn () {
  echo "$@" >&2
}

die () {
  status="$1"
  shift
  warn "$@"
  exit "$status"
}

if [ ! -d vagrant-files ]; then
  die "must be in tools root directory to start the tests"
fi

regex="(.*)/src/github.com/sw/pensando/(.*)"
if [[ `pwd` =~ $regex ]]; then
  GOPATH=${BASH_REMATCH[1]}
  echo "Using GOPATH: $GOPATH"
else
  die "Current directory is not a gopath: something like .../src/github.com/sw/pensando/..."
fi

# change the working directory to dev
if [ ! -d dev ]; then
  die "unable to find 'dev' directory; not sure if 'bringup-dev.sh' was run"
fi

cd dev
if ! vagrant destroy -f; then
  die "failed to clean up vagrant vms"
fi

exit 0
