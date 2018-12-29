# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   = asicerrord.bin
MODULE_SOLIBS   = sdkpal delphisdk logger halproto shmmgr
MODULE_LDLIBS   = rt dl pthread thread :libprotobuf.so.14 ev \
                  grpc++ grpc++_reflection grpc++_unsecure grpc_unsecure
include ${MKDEFS}/post.mk
