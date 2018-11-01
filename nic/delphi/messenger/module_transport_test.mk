# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk

MODULE_TARGET   = delphi_transport_test.gtest
MODULE_ARCH     = x86_64
MODULE_ARLIBS   = delphimessenger delphiutils messengerproto delphiproto
MODULE_LDLIBS   = ${NIC_THIRDPARTY_GOOGLE_LDLIBS} rt ev
MODULE_SRCS     = ${MODULE_SRC_DIR}/tcp_transport_test.cc

include ${MKDEFS}/post.mk
