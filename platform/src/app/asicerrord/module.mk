# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = asicerrord.bin
MODULE_PIPELINE = iris
MODULE_SOLIBS   = sdkpal delphisdk logger halproto shmmgr
MODULE_LDLIBS   = rt dl :libprotobuf.so.14 ev grpc++
include ${MKDEFS}/post.mk
