# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk

MODULE_TARGET   = delphi_messenger_test.gtest
MODULE_ARLIBS   = delphimessenger delphiutils messengerproto delphiproto
MODULE_LDLIBS   = ${NIC_THIRDPARTY_GOOGLE_LDLIBS} rt ev
MODULE_SRCS     = ${MODULE_SRC_DIR}/messenger_test.cc

include ${MKDEFS}/post.mk
