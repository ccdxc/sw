# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = cap_model.bin
MODULE_ASIC     = capri
MODULE_ARCH     = x86_64
MODULE_LDPATHS  = $(abspath ${MODULE_DIR}/../../libs)
MODULE_LDLIBS   = model_cc capsim common_cc top_csr_compile sknobs \
                  asmsym mpuobj capisa isa Judy python2.7 zmq \
                  gmp gmpxx pthread dl util \
                  :libcrypto.so.1.0.2k stdc++ m
include ${MKDEFS}/post.mk
