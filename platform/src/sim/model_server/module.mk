# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_ARCH     := x86_64
MODULE_TARGET   := platform_model_server.bin
MODULE_SOLIBS   := pciemgr_if pciemgr pciemgrutils pciehdevices pcieport pcietlp cfgspace \
                    intrutils pal misc simlib simdev halproto
MODULE_LDPATHS  := ${TOPDIR}/nic/model_sim/libs/
MODULE_LDLIBS   := z model_cc capsim common_cc top_csr_compile sknobs \
                  asmsym mpuobj capisa isa Judy python2.7 zmq \
                  gmp gmpxx pthread dl util \
                  :libcrypto.so.1.0.2k stdc++ m \
                  ${NIC_THIRDPARTY_GOOGLE_LDLIBS}
include ${MKDEFS}/post.mk
