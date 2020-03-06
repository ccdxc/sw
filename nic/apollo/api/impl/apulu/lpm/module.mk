# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = liblpmitree_apulu.lib
MODULE_SRCS     = ${MODULE_SRC_DIR}/../../lpm/lpm.cc \
                  ${MODULE_SRC_DIR}/../../lpm/lpm_priv.cc \
                  ${MODULE_SRC_DIR}/../../lpm/lpm_ipv4_acl.cc \
                  ${MODULE_SRC_DIR}/../../lpm/lpm_ipv4_route.cc \
                  ${MODULE_SRC_DIR}/../../lpm/lpm_ipv4_sip.cc \
                  ${MODULE_SRC_DIR}/../../lpm/lpm_ipv6_acl.cc \
                  ${MODULE_SRC_DIR}/../../lpm/lpm_ipv6_route.cc \
                  ${MODULE_SRC_DIR}/../../lpm/lpm_ipv6_sip.cc \
                  ${MODULE_SRC_DIR}/../../lpm/lpm_proto_dport.cc \
                  ${MODULE_SRC_DIR}/../../lpm/lpm_sport.cc \
                  $(wildcard ${MODULE_SRC_DIR}/*.cc)
MODULE_PIPELINE = apulu
include ${MKDEFS}/post.mk
