# {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

cc_library(
    name = "pnso_drv",
    srcs = [
        "pnso_mpool.c",
        "pnso_api.c",
        "pnso_req.c",
        "pnso_chain.c",
        "pnso_cp.c",
        "pnso_dc.c",
        "pnso_hash.c",
        "pnso_chksum.c",
        "pnso_cpdc.c",
        "pnso_cpdc_cmn.c",
        "pnso_encrypt.c",
        "pnso_decrypt.c",
        "pnso_xts_cmn.c",
        "pnso_xts.c",
        "pnso_decompact.c",
        "pnso_seq.c",
        "pnso_seq_hw.c",
        "pnso_seq_model.c",
    ],
    includes = [
	"../common"
    ],
    deps = [
	"//storage/offload:pnso_pbuf",
	"//storage/offload:osal",
	"//storage/offload/include:pnso",
    ],
    copts = [
        "-g",
        "-xc",
    ],
    hdrs = [
        "pnso_mpool.h",
        "pnso_chain.h",
        "pnso_cpdc.h",
        "pnso_cpdc_cmn.h",
        "pnso_xts.h",
        "pnso_xts_cmn.h",
        "pnso_seq_ops.h",
        "pnso_seq.h",
    ],
)
