include ${MKDEFS}/pre.mk
MODULE_TARGET        = libsysmgreventsproto.so
MODULE_PIPELINE      = iris gft
MODULE_INCS          = /usr/local/include \
                       ${TOPDIR}/vendor/github.com/gogo/protobuf/gogoproto \
                       ${TOPDIR}/nic/hal/third-party/google/include \
                       ${TOPDIR}/hal/third-party/grpc/include
MODULE_FLAGS         = -O3
MODULE_EXCLUDE_FLAGS = -O2
MODULE_PREREQS       = sysmgr_events.proto gogo.proto
MODULE_SRCS          = ${BLD_PROTOGEN_DIR}/sysmgr_events.pb.cc \
                       ${BLD_PROTOGEN_DIR}/gogo.pb.cc
include ${MKDEFS}/post.mk
