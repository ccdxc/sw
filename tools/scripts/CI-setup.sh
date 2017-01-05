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
  die "must be in tools root directory to run the tests"
fi

# create testdev directory
if [ ! -d testdev ]; then
  if ! mkdir testdev; then
    die "unable to create 'testdev' directory"
  fi
  cd testdev
else
  # clean up any remnants from previous run
  cd testdev
  vagrant destroy -f
fi
mkdir -p bin

# bring the test VMs up
if [ ! -f Vagrantfile ]; then
if ! cp ../vagrant-files/Vagrantfile.dev Vagrantfile; then
  die "failed to copy Vagrantfile.dev"
fi
fi

if ! vagrant up --no-provision; then
  die "failed to bring up vagrant vms"
fi

# do parallel provisioning of vagrant boxes
vagrant status | grep node | awk '{print $1}' | xargs -P6 -I"BOXNAME"  sh -c 'vagrant provision BOXNAME'
