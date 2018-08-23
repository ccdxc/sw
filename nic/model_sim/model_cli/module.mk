# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MAKEDEFS}/pre.mk
MODULE_TARGET = model_sim_cli
MODULE_SOLIBS = modelclient
MODULE_LDLIBS = zmq
include ${MAKEDEFS}/post.mk
