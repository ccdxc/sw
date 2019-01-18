#!/bin/bash

# For test framework compatibility with other driver source packages, we have
# this build.sh.  Running build.sh is equivalent to running make in the drivers
# directory of this package.

DIR=$(dirname "$0")
DIR=$(readlink -f "$DIR")

make -j12 -C "$DIR/drivers" || exit
