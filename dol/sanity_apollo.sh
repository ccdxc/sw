#! /bin/bash

export PIPELINE=apollo

# initial setup
source ./sanity_setup.sh
setup ${PIPELINE}

build

# run all apollo DOL dry runs
DryRunSanity mplsoudp networking
DryRunSanity vxlan networking
DryRunSanity lpm lpm
DryRunSanity rfc rfc
#DryRunSanity mirror mirror

# end of script
clean_exit
