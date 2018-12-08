# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = vcn_test.gtest
MODULE_PIPELINE = apollo
MODULE_ARCH     = x86_64
MODULE_SOLIBS   = ociframework ocicore ociapi ocitestutils \
				  ${NIC_${PIPELINE}_P4PD_SOLIBS} \
				  ${NIC_SDK_SOLIBS} \
				  ${NIC_${PIPELINE}_P4PD_SOLIBS} \
				  p4pd sdkp4 sdkp4utils \
				  p4pd_common_p4plus_rxdma pdcommon \
				  p4pd_common_p4plus_txdma asicpd pdcapri capri_csrint \
				  capri_csr_cpu_hal sdkcapri bm_allocator sdkplatformutils \
				  hal_mock halproto mtrack
MODULE_LDLIBS   =  ${NIC_COMMON_LDLIBS} \
				   ${NIC_CAPSIM_LDLIBS} \
				   ${NIC_THIRDPARTY_GOOGLE_LDLIBS} \
				   ${NIC_THIRDPARTY_SSL_LDLIBS}
include ${MKDEFS}/post.mk
