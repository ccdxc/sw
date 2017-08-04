#!/bin/bash

warn () {
  echo "$@" >&2
}

die () {
  status="$1"
  shift
  warn "$@"
  exit "$status"
}

if [ ! -d tools/vagrant-files ]; then
  die 1 "Must be in root directory to clean dev environment."
fi

regex="(.*)/src/github.com/pensando/sw(.*)"
if [[ `pwd` =~ $regex ]]; then
  GOPATH=${BASH_REMATCH[1]}
  echo "Using GOPATH: $GOPATH"
else
  die 1 "Current directory is not a gopath: something like $regex"
fi

# sanity checks and warnings before destroy
if [ ! -f Vagrantfile ]; then
  die 1 "unable to find 'Vagrantfile'; not sure if bringup was ever done; pls check 'vagrant global-status' "
fi

if ! vagrant destroy -f; then
  die 2 "failed to clean up vagrant vms"
fi

rm -f Vagrantfile

exit 0
