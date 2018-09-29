# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET       = agent.proto
MODULE_GEN_TYPES    = GO
MODULE_PIPELINE     = iris
MODULE_INCS         = ${MODULE_DIR} \
                      ${TOPDIR}/vendor \
                      ${GOPATH}/src \
                      ${TOPDIR}/vendor/github.com/gogo/protobuf/protobuf \
                      ${TOPDIR}/vendor/github.com/pensando/grpc-gateway/third_party \
                      ${TOPDIR}/vendor/github.com/pensando/grpc-gateway/third_party/googleapis \
                      /usr/local/include
MODULE_PROTOC_GOFAST_OPTS = --gofast_out=grpc:${MODULE_DIR}
include ${MKDEFS}/post.mk
