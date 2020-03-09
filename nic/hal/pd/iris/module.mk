# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   = libpd_iris.lib
MODULE_PIPELINE = iris
MODULE_PREREQS  = iris.p4bin \
                  common_p4plus_txdma.p4bin common_p4plus_rxdma.p4bin \
                  proxy.p4bin adminq.p4bin app_redir_p4plus.p4bin \
                  cpu_p4plus.p4bin eth.p4bin gc.p4bin ipfix.p4bin \
                  ipsec_p4plus_esp_v4_tun_hton.p4bin \
                  ipsec_p4plus_esp_v4_tun_ntoh.p4bin \
                  p4pt.p4bin tls.p4bin rdma.p4bin smbdc.p4bin \
                  storage.p4bin iris_nvme.p4bin
MODULE_SRCS     := $(wildcard ${MODULE_SRC_DIR}/*.cc) \
                   $(wildcard ${MODULE_SRC_DIR}/aclqos/*.cc) \
                   $(wildcard ${MODULE_SRC_DIR}/lif/*.cc) \
                   $(wildcard ${MODULE_SRC_DIR}/mcast/*.cc) \
                   $(wildcard ${MODULE_SRC_DIR}/nw/*.cc) \
                   $(wildcard ${MODULE_SRC_DIR}/nvme/*.cc) \
                   $(wildcard ${MODULE_SRC_DIR}/tcp_proxy/*.cc) \
                   $(wildcard ${MODULE_SRC_DIR}/debug/*.cc) \
                   $(wildcard ${MODULE_SRC_DIR}/firewall/*.cc) \
                   $(wildcard ${MODULE_SRC_DIR}/dos/*.cc) \
                   $(wildcard ${MODULE_SRC_DIR}/telemetry/*.cc) \
                   $(wildcard ${MODULE_SRC_DIR}/event/*.cc) \
                   $(wildcard ${MODULE_SRC_DIR}/../common_p4plus/*.cc) \
                   $(wildcard ${MODULE_SRC_DIR}/flow/*.cc)
ifeq ($(ARCH),x86_64)
MODULE_SRCS     := ${MODULE_SRCS} \
                   $(wildcard ${MODULE_SRC_DIR}/ipsec/*.cc) \
                   $(wildcard ${MODULE_SRC_DIR}/rdma/*.cc) \
                   $(wildcard ${MODULE_SRC_DIR}/internal/*.cc)
else
MODULE_SRCS     := ${MODULE_SRCS} \
                   ${MODULE_SRC_DIR}/internal/gccb_pd.cc \
                   ${MODULE_SRC_DIR}/internal/crypto_keys_pd.cc \
                   ${MODULE_SRC_DIR}/internal/cpucb_pd.cc \
                   ${MODULE_SRC_DIR}/internal/p4pt_pd.cc \
                   ${MODULE_SRC_DIR}/internal/proxyrcb_pd.cc \
                   ${MODULE_SRC_DIR}/internal/proxyccb_pd.cc \
                   ${MODULE_SRC_DIR}/internal/swphv_pd.cc \
                   ${MODULE_SRC_DIR}/internal/rawrcb_pd.cc \
                   ${MODULE_SRC_DIR}/internal/rawccb_pd.cc \
                   ${MODULE_SRC_DIR}/internal/system_pd.cc \
                   ${MODULE_SRC_DIR}/internal/tcpcb_pd.cc \
                   ${MODULE_SRC_DIR}/internal/tlscb_post_pd.cc \
                   ${MODULE_SRC_DIR}/internal/tlscb_pd.cc \
                   ${MODULE_SRC_DIR}/internal/copp_pd.cc
endif
MODULE_INCS     = ${BLD_PROTOGEN_DIR}
MODULE_SOLIBS   = ${NIC_FTL_LIBS}
include ${MKDEFS}/post.mk
