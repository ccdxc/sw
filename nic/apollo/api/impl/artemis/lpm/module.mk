# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = liblpmitree_artemis.so
MODULE_SRCS     = ${MODULE_SRC_DIR}/../../lpm/lpm.cc \
                  ${MODULE_SRC_DIR}/../../lpm/lpm_priv.cc \
                  ${MODULE_SRC_DIR}/../../lpm/lpm_ipv4_acl.cc \
                  ${MODULE_SRC_DIR}/../../lpm/lpm_ipv4_meter.cc \
                  ${MODULE_SRC_DIR}/../../lpm/lpm_ipv4_peer_route.cc \
                  ${MODULE_SRC_DIR}/../../lpm/lpm_ipv4_route.cc \
                  ${MODULE_SRC_DIR}/../../lpm/lpm_ipv4_sip.cc \
                  ${MODULE_SRC_DIR}/../../lpm/lpm_ipv4_tag.cc \
                  ${MODULE_SRC_DIR}/../../lpm/lpm_ipv6_acl.cc \
                  ${MODULE_SRC_DIR}/../../lpm/lpm_ipv6_meter.cc \
                  ${MODULE_SRC_DIR}/../../lpm/lpm_ipv6_peer_route.cc \
                  ${MODULE_SRC_DIR}/../../lpm/lpm_ipv6_route.cc \
                  ${MODULE_SRC_DIR}/../../lpm/lpm_ipv6_sip.cc \
                  ${MODULE_SRC_DIR}/../../lpm/lpm_ipv6_tag.cc \
                  ${MODULE_SRC_DIR}/../../lpm/lpm_proto_dport.cc \
                  $(wildcard ${MODULE_SRC_DIR}/*.cc)
MODULE_PIPELINE = artemis
include ${MKDEFS}/post.mk
