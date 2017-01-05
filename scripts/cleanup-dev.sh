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

if [ ! -d tools/vagrant-files ]; then
  die "can't find tools/vagrant-files, must not be in root directory... exiting"
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
  die 2 "unable to find 'Vagrantfile'; not sure if bringup was ever done; pls check 'vagrant global-status' "
fi

if ! vagrant destroy -f; then
  die "failed to clean up vagrant vms"
fi

exit 0
