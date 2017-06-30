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

# perform ping test between two VMs
vagrant ssh -c "ping -c 5 node2" node1
ret=$?
if [ $ret == "" ]; then
  die 1 "Ping test failed!!"
fi

vagrant ssh -c "go version" node1
ret=$?
if [ $ret == "" ]; then
  die 1 "Go not installed!!"
fi

exit 0
