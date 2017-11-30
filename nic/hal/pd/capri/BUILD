package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

cc_library(
    name = "capri_includes",
    srcs = [],
    hdrs = [
        "capri_hbm.hpp",
        "capri_loader.h",
        "capri_barco_rings.hpp",
        "capri_tbl_rw.hpp",
        "capri_barco_asym_apis.hpp",
        "capri_barco_sym_apis.hpp",
        "capri_barco_crypto.hpp",
    ],
)

capri_util_deps = [
    "//nic/utils/bm_allocator:bm_allocator",
    "//nic/utils/slab:slab",
    "//nic/utils/indexer:indexer",
    "//nic/utils/catalog:catalog",
]

capri_pd_deps = [
    "//nic/hal/pd/capri/csr:capricsr",
    "//nic/hal/pd/common:pdcommon",
    "//nic/hal/pd/iris:iris_includes",
]

cc_library(
    name = "capri",

    srcs = [
        "capri_tbl_rw.cc",
        "capri_loader.cc",
        "capri_qstate.cc",
        "capri_config.cc",
        "capri_hbm.cc",
        "capri_asic_rw.cc",
        "capri_barco_crypto.cc",
        "capri_tm_rw.cc",
        "capri_init.cc",
        "capri_repl.cc",
        "capri_txs_scheduler.cc",
        "capri_pxb_pcie.cc",
        "capri_barco_rings.cc",
        "capri_barco_res.cc",
        "capri_barco_asym_apis.cc",
        "capri_barco_asym_tests.cc",
        "capri_barco_sym_apis.cc",
        "capri_barco_sym_tests.cc"
    ],

    hdrs = [
    ] + glob(["*.hpp"]) + glob(["*.h"]),

    deps = [
        "//nic:asic_includes",
        "//nic:gen_includes",
        "//nic:gen_proto_includes",
        "//nic:capricsr_int",
        "//nic/include:base_includes",
        "//nic/utils/block_list:block_list",
        "//nic/model_sim/include:modelsim_includes",
        "//nic/p4/nw/include:p4_nw_includes",
        "//nic/third-party/spdlog",
    ]
    + capri_util_deps
    + capri_pd_deps,

    copts = [
        "-DCAPRI_HAL"
    ],

    alwayslink = 1,
)
