#!/usr/bin/tcsh

setenv ASIC_SRC "$WORKSPACE/src/github.com/pensando/sw/nic/asic"
setenv ASIC_GEN "$WORKSPACE/src/github.com/pensando/sw/nic/asic_gen"
rm -rf $ASIC_GEN
mkdir -p $ASIC_GEN
setenv ASIC_COMMON /home/asic
source $ASIC_COMMON/templates/cshrc_include

#rm -rf $ASIC_GEN/*
cd $ASIC_SRC/capri/verif/top/env && runtest -ngrid -test core_basic,axi_fd_access_ar -run_args '+UVM_MAX_QUIT_COUNT=1 +PLOG_MAX_QUIT_COUNT=1 +UVM_VERBOSITY=UVM_LOW' -partcomp_new -queue=sim_int


cd "$WORKSPACE/src/github.com/pensando/sw/nic/"
./run.py --topo up2up --feature networking --rtl --skipverify --regress

