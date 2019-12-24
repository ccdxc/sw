#! /bin/bash

export PIPELINE=artemis

# initial setup
source ./sanity_setup.sh
setup ${PIPELINE}

build

# run all artemis DOL dry runs
DryRunSanity vxlan networking

# end of script
clean_exit