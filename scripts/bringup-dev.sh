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

if [ $# -ne 1 ]; then
  die 1 "Missing vagrantfile name.  Usage: $0 <vagrantfile>"
fi

vagrantfile=$1

if [ ! -d tools/vagrant-files ]; then
  die 1 "Must be in root directory to start dev environment."
fi

# validate paths before starting VMs
regex="(.*)/src/github.com/pensando/sw(.*)"
if [[ `pwd` =~ $regex ]]; then
  export GOPATH=${BASH_REMATCH[1]}
  echo "Using GOPATH: $GOPATH"
else
  die 1 "Current directory is not a gopath: something like $regex."
fi

if [ -f Vagrantfile ]; then
  die 1 "Local Vagrantfile exists; 'make dev-clean' first."
fi

# bring the test VMs up
vagrantpath=tools/vagrant-files/$vagrantfile
if ! cp $vagrantpath Vagrantfile; then
  die 1 "$vagrantpath: copy to Vagrantfile failed."
fi

if [ ! -d bin ]; then
  if ! mkdir bin; then
    die 1 "Unable to create 'bin' directory."
  fi
fi

if ! vagrant up; then
  die 2 "Failed to bring up vagrant vms"
fi

exit 0
