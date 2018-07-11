# {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

cc_library(
    name = "pnso_drv",
    srcs = [
        "pnso_dpool.c",
        "pnso_cp.c",
        "pnso_dc.c",
        "pnso_chksum.c",
        "pnso_hash.c",
        "pnso_cpdc.c",
        "pnso_cpdc_cmn.c",
        "pnso_encrypt.c",
        "pnso_decrypt.c",
        "pnso_xts_cmn.c",
        "pnso_xts.c",
        "pnso_decompact.c",
    ],
    includes = [
        "src/common",
    ],
    deps = [
	"//storage/offload/src/osal:osal",
	"//storage/offload/include:pnso",
#	"//storage/offload/src/common:pnso_common",
    ],
    copts = [
        "-g",
        "-xc",
    ],
    hdrs = [
        "pnso_dpool.h",
        "pnso_chain.h",
        "pnso_cpdc.h",
        "pnso_cpdc_cmn.h",
        "pnso_xts.h",
        "pnso_xts_cmn.h",
    ],
)
