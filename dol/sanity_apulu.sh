#! /bin/bash

export PIPELINE=apulu

# initial setup
source ./sanity_setup.sh
setup ${PIPELINE}

build

# run all apulu DOL dry runs
DryRunSanity hostvxlan networking

# end of script
clean_exit