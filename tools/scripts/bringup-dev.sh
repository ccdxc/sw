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

if [ ! -d vagrant-files ]; then
  die "must be in tools root directory to start the tests"
fi

regex="(.*)/src/github.com/pensando/sw/(.*)"
if [[ `pwd` =~ $regex ]]; then
  export GOPATH=${BASH_REMATCH[1]}
  echo "Using GOPATH: $GOPATH"
else
  die "Current directory is not a gopath: something like .../src/github.com/sw/pensando/..."
fi

# create dev directory
if [ ! -d dev ]; then
  if ! mkdir dev; then
    die "unable to create 'dev' directory"
  fi
  cd dev
else
  cd dev
  if ! vagrant status; then
    warn "the dev setup seems already up and running, not doing anything"
    exit 0
  fi
fi

mkdir -p bin

# bring the test VMs up
if ! cp ../vagrant-files/Vagrantfile.dev Vagrantfile; then
  die "failed to find/copy Vagrantfile.dev"
fi

if ! vagrant up --no-provision; then
  die "failed to bring up vagrant vms"
fi

# do parallel provisioning of vagrant boxes
vagrant status | grep node | awk '{print $1}' | xargs -P6 -I"BOXNAME"  sh -c 'vagrant provision BOXNAME'

exit 0
