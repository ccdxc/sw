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
  die "must be root directory to start dev environment"
fi

# validate paths before starting VMs
regex="(.*)/src/github.com/pensando/sw(.*)"
if [[ `pwd` =~ $regex ]]; then
  export GOPATH=${BASH_REMATCH[1]}
  echo "Using GOPATH: $GOPATH"
else
  die 1 "Current directory is not a gopath: something like $regex"
fi

# bring the test VMs up
if ! cp tools/vagrant-files/Vagrantfile.dev Vagrantfile; then
  die 2 "failed to find/copy Vagrantfile.dev"
fi

if [ ! -d bin ]; then
  if ! mkdir bin; then
    die 4 "unable to create 'bin' directory"
  fi
fi

if ! vagrant up; then
  die "failed to bring up vagrant vms"
fi

exit 0
