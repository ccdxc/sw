# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET = model_sim_cli.bin
MODULE_PIPELINE = iris gft
MODULE_SOLIBS = model_client
MODULE_LDLIBS = zmq ${NIC_THIRDPARTY_SSL_LDLIBS}
include ${MKDEFS}/post.mk
