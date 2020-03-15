# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = ncsid
MODULE_PIPELINE := iris
MODULE_PREREQS  := nicmgr.proto delphi.proto

MODULE_SOLIBS   = ncsi logger nicmgrproto delphisdk halproto sdkfru sdkplatformutils device bm_allocator shmmgr catalog
MODULE_LDLIBS   := dl pal evutils ${NIC_COMMON_LDLIBS} \
					${NIC_THIRDPARTY_GOOGLE_LDLIBS}
MODULE_FLAGS    = -DCAPRI_SW

MODULE_INCS     := ${MODULE_SRC_DIR} ${BLD_PROTOGEN_DIR}

include ${MKDEFS}/post.mk
